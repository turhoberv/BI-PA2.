#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <compare>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std::literals;
#endif /* __PROGTEST__ */

// nejhorsi progtest, co jsem kdy vytvoril, pokud tohle cte nekdo na code
// review, hluboce se za muj vytvor omlouvam

class CTimeStamp {
public:
  int year() const { return y; }

  int month() const { return mon; }

  int day() const { return d; }

  int hour() const { return h; }

  int minute() const { return min; }

  std::strong_ordering operator<=>(const CTimeStamp &other) const {
    return std::make_tuple(this->year(), this->month(), this->day(),
                           this->hour(), this->minute()) <=>
           std::make_tuple(other.year(), other.month(), other.day(),
                           other.hour(), other.minute());
  }

  bool isLeapYear(int y) const {
    if ((y % 100 != 0 && y % 4 == 0) || y % 400 == 0)
      return true;
    return false;
  }

  bool isValid(int y, int mon, int day, int hour, int min) const {
    if (y < 1850 || y > 2150)
      return false;
    if (mon <= 0 || mon > 12)
      return false;
    static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};

    int maxDays = daysInMonth[mon];
    if (mon == 2 && isLeapYear(y)) {
      maxDays++;
    }

    if (day <= 0 || day > maxDays)
      return false;
    if (hour < 0 || hour > 23)
      return false;
    if (min < 0 || min > 59)
      return false;
    return true;
  }

  CTimeStamp(int y, int mon, int d, int h, int min) {
    if (isValid(y, mon, d, h, min)) {
      this->y = y;
      this->mon = mon;
      this->d = d;
      this->h = h;
      this->min = min;
      this->inMins = toMinutes();
    } else
      throw std::runtime_error("invalid date/time");
  }

  long long inMinutes() const { return inMins; }

  CTimeStamp(const CTimeStamp &other) {
    this->y = other.year();
    this->mon = other.month();
    this->d = other.day();
    this->h = other.hour();
    this->min = other.minute();
    this->inMins = other.inMinutes();
  }

  ~CTimeStamp() = default;

private:
  int y, mon, d, h, min;
  long long inMins;
  long long toMinutes() {
    int totalDays = 0;
    for (int i = 1900; i < y; i++) {
      totalDays += isLeapYear(i) ? 366 : 365;
    }
    static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
    for (int i = 1; i < mon; i++) {
      totalDays += daysInMonth[i];
    }
    if (mon > 2 && isLeapYear(y)) {
      totalDays++;
    }
    totalDays += d - 1;
    long long totalMinutes = totalDays * 24 * 60;
    totalMinutes += h * 60 + min;
    return totalMinutes;
  }
};

class CAuditFilter {
public:
  CAuditFilter(const std::string &zoneName) : zoneName(zoneName) {}

  CAuditFilter &notBefore(int y, int mon, int d, int h, int min) {
    m_notBefore = CTimeStamp(y, mon, d, h, min);
    return *this;
  }

  CAuditFilter &notAfter(int y, int mon, int d, int h, int min) {
    m_notAfter = CTimeStamp(y, mon, d, h, min);
    return *this;
  }
  std::optional<CTimeStamp> getNotBefore() { return m_notBefore; }
  std::optional<CTimeStamp> getNotAfter() { return m_notAfter; }

  std::string getZoneName() { return zoneName; }

private:
  std::string zoneName;
  std::optional<CTimeStamp> m_notBefore, m_notAfter;
};

class CLogItem {
public:
  CLogItem(std::string gate, CTimeStamp ts) : zone(gate), timeStamp(ts) {}
  std::string getGate() const { return zone; }
  CTimeStamp getTimeStamp() const { return timeStamp; }

private:
  std::string zone;
  CTimeStamp timeStamp;
};

class CVisitorLog {
public:
  std::set<std::string> search(CAuditFilter filter) const {

    std::set<std::string> result;
    long long notAfterInMins = std::numeric_limits<long long>::max();
    long long notBeforeInMins = -1;
    if (filter.getNotBefore()) {
      notBeforeInMins = filter.getNotBefore()->inMinutes();
    }
    if (filter.getNotAfter()) {
      notAfterInMins = filter.getNotAfter()->inMinutes();
    }

    for (const auto &personLog : m_PersonLogs) {

      auto sortedLogs = personLog.second;

      for (size_t i = 0; i < sortedLogs.size(); i += 2) {
        CLogItem entry = sortedLogs[i];

        // int distIn = getDistance(entry.getGate(), filter.getZoneName());
        int distIn = -1;

        if (entry.getGate() == filter.getZoneName()) {
          distIn = 0;
        } else if (m_Distances.contains(entry.getGate()) &&
                   m_Distances.at(entry.getGate())
                       .contains(filter.getZoneName())) {
          distIn = m_Distances.at(entry.getGate()).at(filter.getZoneName());
        }

        if (distIn < 0) {
          continue;
        }

        long long tArr = entry.getTimeStamp().inMinutes() + distIn;
        long long tDep = std::numeric_limits<long long>::max();
        // defaultne nastavim, ze clovek neodesel

        // pokud clovek odesel, nastavim mu jeho cas odchodu
        if (i + 1 < sortedLogs.size()) {
          CLogItem exit = sortedLogs[i + 1];
          // int distOut = getDistance(filter.getZoneName(), exit.getGate());
          int distOut = -1;
          if (exit.getGate() == filter.getZoneName()) {
            distOut = 0;
          } else if (m_Distances.contains(filter.getZoneName()) &&
                     m_Distances.at(filter.getZoneName())
                         .contains(exit.getGate())) {
            distOut = m_Distances.at(filter.getZoneName()).at(exit.getGate());
          }
          if (distOut < 0) {
            continue;
          }
          tDep = exit.getTimeStamp().inMinutes() - distOut;
        }

        if (tArr <= tDep) {
          if (std::max(tArr, notBeforeInMins) <=
              std::min(tDep, notAfterInMins)) {
            result.insert(personLog.first);
            break;
          }
        }
      }
    }
    return result;
  }
  CVisitorLog(
      const std::map<std::string, std::map<std::string, int>> &distances) {
    m_Distances = distances;
  }
  bool insert(const std::string &gate, int y, int mon, int d, int h, int min,
              const std::string &name) {
    CTimeStamp ts(y, mon, d, h, min);
    CLogItem item(gate, ts);
    m_PersonLogs[name].emplace_back(item);
    return true;
  }
  bool insert(const std::string &gate, const CTimeStamp &ts,
              const std::string &name) {
    CLogItem item(gate, ts);
    m_PersonLogs[name].emplace_back(item);
    return true;
  }

  void sortLogs() {
    for (auto &log : m_PersonLogs) {
      std::sort(log.second.begin(), log.second.end(),
                [](const CLogItem &a, const CLogItem &b) {
                  return a.getTimeStamp().inMinutes() <
                         b.getTimeStamp().inMinutes();
                });
    }
  }

private:
  std::map<std::string, std::vector<CLogItem>>
      m_PersonLogs; // name, <gate, time>

  // std::map<std::string, std::vector<std::pair<std::string, int>>> base_Graph;
  std::map<std::string, std::map<std::string, int>> m_Distances;
};

class CMilBase {
public:
  CMilBase() {}

  int getDistance(const std::string &from, const std::string &to) const {
    if (from == to)
      return 0;
    if (m_Graph.find(from) == m_Graph.end()) {
      return -1;
    }
    std::map<std::string, int> distances;
    std::deque<std::string> queue;

    queue.push_back(from);
    distances[from] = 0;
    while (!queue.empty()) {
      std::string curr = queue.front();
      queue.pop_front();
      if (curr == to) {
        return distances[curr];
      }
      for (auto neighbor : m_Graph.at(curr)) {
        if (!distances.contains(neighbor.first)) {
          queue.push_back(neighbor.first);
          distances[neighbor.first] =
              distances[curr] + neighbor.second; // funguje jen pro prisnost 1
        }
      }
    }
    return -1;
  }

  void readBase(const std::string &baseFilename) {
    std::ifstream ifs(baseFilename);
    if (!ifs.is_open()) {
      throw std::runtime_error("could not open file");
    }

    int x;
    std::string first, second;

    std::string line;

    while (std::getline(ifs, line)) {
      std::istringstream iss(line);

      while (iss >> first) {
        if (!(iss >> second >> x)) {
          throw std::runtime_error(
              "incomplete line or invalid strictness format");
        }

        if (x < 1 || x > 9) {
          throw std::runtime_error("invalid base strictness");
        }

        m_Graph[first].emplace_back(second, x);
        m_Graph[second].emplace_back(first, x);
      }
    }
    if (!ifs.eof()) {
      throw std::runtime_error("wrong file format >:(");
    }

    for (const auto &from : m_Graph) {
      for (const auto &to : m_Graph) {
        m_Distances[from.first][to.first] = getDistance(from.first, to.first);
      }
    }
  }

  uint16_t swap16(uint16_t v) {
    uint16_t u = v;
    v = v >> 8;
    u = u << 8;
    return v | u;
  }
  uint32_t swap32(uint32_t v) {
    return ((v << 24) & 0xff000000) | ((v << 8) & 0x00ff0000) |
           ((v >> 8) & 0x0000ff00) | ((v >> 24) & 0x000000ff);
  }

  CTimeStamp getTimeFromBinary(uint32_t datetime) {
    uint32_t year = datetime & 0b11111111111100000000000000000000;
    year = year >> 20;

    uint32_t month = datetime & 0b00000000000011110000000000000000;
    month = month >> 16;

    uint32_t day = datetime & 0b00000000000000001111100000000000;
    day = day >> 11;

    uint32_t hour = datetime & 0b00000000000000000000011111000000;
    hour = hour >> 6;

    uint32_t minute = datetime & 0b00000000000000000000000000111111;
    return CTimeStamp(static_cast<int>(year), static_cast<int>(month),
                      static_cast<int>(day), static_cast<int>(hour),
                      static_cast<int>(minute));
  }

  void parseBinary(std::ifstream &ifs, CVisitorLog &log, bool isBigEndian) {
    uint16_t zoneLen;
    if (!ifs.read(reinterpret_cast<char *>(&zoneLen), 2)) {
      throw std::runtime_error("invalid file format");
    }

    if (isBigEndian) {
      zoneLen = swap16(zoneLen);
    }

    std::string zoneName(zoneLen,
                         '\0'); // hotfix aby ifs.read fungovalo se stringem
    if (!ifs.read(&zoneName[0], zoneLen)) {
      throw std::runtime_error("invalid file format");
    }

    uint32_t logCount;

    if (!ifs.read(reinterpret_cast<char *>(&logCount), 4)) {
      throw std::runtime_error("invalid file format");
    }

    if (isBigEndian) {
      logCount = swap32(logCount);
    }

    for (uint32_t i = 0; i < logCount; ++i) {
      uint32_t datetime;

      if (!ifs.read(reinterpret_cast<char *>(&datetime), 4)) {
        throw std::runtime_error("invalid file format");
      }
      if (isBigEndian) {
        datetime = swap32(datetime);
      }
      uint16_t nameLen;
      if (!ifs.read(reinterpret_cast<char *>(&nameLen), 2)) {
        throw std::runtime_error("invalid file format");
      }
      if (isBigEndian) {
        nameLen = swap16(nameLen);
      }
      std::string name(nameLen, '\0');
      if (!ifs.read(&name[0], nameLen)) {
        throw std::runtime_error("invalid file format");
      }
      log.insert(zoneName, getTimeFromBinary(datetime), name);
    }
  }

  CVisitorLog processLog(const std::string &logFilename) {
    std::ifstream ifs(logFilename, std::ios::binary);
    if (!ifs.is_open()) {
      throw std::runtime_error("could not open file");
    }
    char format[4];
    CVisitorLog log(this->m_Distances);
    while (ifs.read(format, 4)) {
      if (memcmp(format, "TEXT", 4) == 0) {
        // text format
        std::string gate;
        int logs;
        if (!(ifs >> gate >> logs)) {
          throw std::runtime_error("invalid file format");
        }
        if (logs < 0) {
          throw std::runtime_error("invalid log count");
        }
        if (!(ifs >> std::ws)) {
          throw std::runtime_error("invalid file format");
        }
        for (int i = 0; i < logs; ++i) {
          std::string line;
          if (!std::getline(ifs, line)) {
            throw std::runtime_error("invalid format");
          }
          int y, mon, d, h, min;
          int offset;
          if (sscanf(line.c_str(), "%d-%d-%d %d:%d%n", &y, &mon, &d, &h, &min,
                     &offset) != 5) {
            throw std::runtime_error("invalid format");
          }
          offset++;
          std::string name = line.substr(offset);
          log.insert(gate, y, mon, d, h, min, name);
        }
      } else if (memcmp(format, "IIII", 4) == 0) {
        // little endian format
        parseBinary(ifs, log, false);
      } else if (memcmp(format, "MMMM", 4) == 0) {
        // big endian format
        parseBinary(ifs, log, true);
      } else {
        throw std::runtime_error("invalid format");
      }
    }

    if (ifs.gcount() > 0) {
      throw std::runtime_error("invalid format");
    }
    log.sortLogs();
    return log;
  }

private:
  // std::vector<std::tuple<std::string, std::string, int>> m_gates;
  std::map<std::string, std::vector<std::pair<std::string, int>>> m_Graph;
  std::map<std::string, std::map<std::string, int>> m_Distances;
};

#ifndef __PROGTEST__
void basicTests(const CVisitorLog &log) {
  assert(log.search(CAuditFilter("headquarters")) ==
         (std::set<std::string>{"Alice Cooper", "George Peterson",
                                "Henry Montgomery", "Jane Bush", "John Smith",
                                "Tim Cook", "Robert Smith"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")) ==
         (std::set<std::string>{"Alice Cooper", "Henry Montgomery", "Jane Bush",
                                "John Smith", "Robert Smith"}));
  assert(log.search(
             CAuditFilter("flyingSaucerHangar").notAfter(2026, 3, 10, 8, 0)) ==
         (std::set<std::string>{"Henry Montgomery", "Robert Smith"}));
  assert(
      log.search(
          CAuditFilter("flyingSaucerHangar").notBefore(2026, 3, 11, 12, 0)) ==
      (std::set<std::string>{"Henry Montgomery", "Jane Bush", "John Smith"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2026, 3, 10, 9, 0)
                        .notAfter(2026, 3, 10, 13, 0)) ==
         (std::set<std::string>{"Alice Cooper", "Henry Montgomery", "Jane Bush",
                                "John Smith"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2026, 3, 10, 9, 5)
                        .notAfter(2026, 3, 10, 9, 5)) ==
         (std::set<std::string>{"Henry Montgomery"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2026, 3, 10, 9, 6)
                        .notAfter(2026, 3, 10, 9, 6)) ==
         (std::set<std::string>{"Henry Montgomery", "John Smith"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2026, 3, 10, 9, 24)
                        .notAfter(2026, 3, 10, 9, 24)) ==
         (std::set<std::string>{"Alice Cooper", "Henry Montgomery",
                                "John Smith"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2026, 3, 10, 9, 25)
                        .notAfter(2026, 3, 10, 9, 25)) ==
         (std::set<std::string>{"Alice Cooper", "Henry Montgomery"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2024, 2, 1, 0, 0)
                        .notAfter(2024, 3, 31, 0, 0)) ==
         (std::set<std::string>{"Robert Smith", "Henry Montgomery"}));
  assert(log.search(CAuditFilter("flyingSaucerHangar")
                        .notBefore(2025, 2, 1, 0, 0)
                        .notAfter(2025, 3, 31, 0, 0)) ==
         (std::set<std::string>{"Henry Montgomery"}));
  assert(log.search(CAuditFilter("privateParking")) ==
         (std::set<std::string>{"<classified>"}));
}

int main() {
  class CMilBase b;
  b.readBase("base.txt");

  for (const char *fn :
       std::initializer_list<const char *>{"in1.log", "in2.log", "in3.log"})
    basicTests(b.processLog(fn));
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */