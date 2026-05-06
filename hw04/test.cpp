#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include <iterator>
#endif /* __PROGTEST */

#ifndef __PROGTEST__
class CTimeStamp
{
public:
  CTimeStamp(int year, int month, int day, int hour, int minute, int sec) :
    year(year), month(month), day(day), hour(hour), min(minute), sec(sec) {
  }
  std::strong_ordering operator <=> (const CTimeStamp& x) const {
    return std::tie(year, month, day, hour, min, sec) <=> std::tie(x.year, x.month, x.day, x.hour, x.min, x.sec);

  }
  bool operator == (const CTimeStamp& x) const {
    return std::tie(year, month, day, hour, min, sec) == std::tie(x.year, x.month, x.day, x.hour, x.min, x.sec);

  }
  friend std::ostream& operator << (std::ostream& os, const CTimeStamp& x);
private:
  int year, month, day, hour, min, sec;
};


std::ostream& operator << (std::ostream& os, const CTimeStamp& x) {
  os.fill('0');
  os << std::setw(4) << x.year
    << '-' << std::setw(2) << x.month
    << '-' << std::setw(2) << x.day
    << ' ' << std::setw(2) << x.hour
    << ':' << std::setw(2) << x.min
    << ':' << std::setw(2) << x.sec;

  return os;
}
//=================================================================================================
class CMailBody
{
public:
  CMailBody(int size, const char data[]) : m_Size(size) {
    m_Data = new char[size];
    memcpy(m_Data, data, size);
  }

  CMailBody(const CMailBody& other) : m_Size(other.m_Size) {
    m_Data = new char[m_Size];
    memcpy(this->m_Data, other.m_Data, m_Size);
  }

  CMailBody& operator = (const CMailBody& other) {
    if (this == &other) {
      return *this;
    }
    this->m_Size = other.m_Size;
    delete[] this->m_Data;
    m_Data = new char[m_Size];
    memcpy(this->m_Data, other.m_Data, m_Size);
    return *this;
  }

  // copy cons/op=/destructor is correctly implemented in the testing environment
  friend std::ostream& operator << (std::ostream& os, const CMailBody& x)
  {
    return os << "mail body: " << x.m_Size << " B";
  }
  ~CMailBody() {
    delete[] this->m_Data;
  }
private:
  int            m_Size;
  char* m_Data;
};
//=================================================================================================
class CAttach
{
public:
  CAttach(int x) : m_X(x)
  {
  }
  void addRef()
  {
    m_RefCnt++;
  }
  void release()
  {
    if (!--m_RefCnt)
      delete this;
  }
private:
  int            m_X;
  int            m_RefCnt = 1;
  CAttach(const CAttach& x);
  CAttach& operator =                              (const CAttach& x);
  ~CAttach() = default;
  friend std::ostream& operator <<                      (std::ostream& os,
    const CAttach& x)
  {
    return os << "attachment: " << x.m_X << " B";
  }
};
//=================================================================================================
#endif /* __PROGTEST__, DO NOT remove */


class CMail
{
public:
  CMail(const CTimeStamp& timeStamp, const std::string& from, const CMailBody& body, CAttach* attach = nullptr) : m_TimeStamp(timeStamp), m_From(from), m_Body(body) {
    this->m_Attach = attach;
    if (this->m_Attach) this->m_Attach->addRef();
  }

  CMail(const CMail& other) : m_TimeStamp(other.m_TimeStamp), m_From(other.m_From), m_Body(other.m_Body) {
    this->m_Attach = other.m_Attach;
    if (this->m_Attach) this->m_Attach->addRef();
  }

  CMail(CMail&& other) noexcept : m_TimeStamp(std::move(other.m_TimeStamp)), m_From(std::move(other.m_From)), m_Body(std::move(other.m_Body)), m_Attach(other.m_Attach) {
    other.m_Attach = nullptr;
  }

  CMail& operator = (const CMail& other) {
    if (this == &other) return *this;
    this->m_TimeStamp = other.m_TimeStamp;
    this->m_From = other.m_From;
    this->m_Body = other.m_Body;
    if (this->m_Attach) this->m_Attach->release();
    this->m_Attach = other.m_Attach;
    if (this->m_Attach) this->m_Attach->addRef();
    return *this;
  }

  CMail& operator = (CMail&& other) noexcept {
    if (this == &other) return *this;
    this->m_TimeStamp = std::move(other.m_TimeStamp);
    this->m_From = std::move(other.m_From);
    this->m_Body = std::move(other.m_Body);
    this->m_Attach = other.m_Attach;
    other.m_Attach = nullptr;
    return *this;
  }

  std::strong_ordering operator <=> (const CMail& other) const {
    return this->timeStamp() <=> other.timeStamp();
  }
  const std::string& from() const {
    return this->m_From;
  }
  const CMailBody& body() const {
    return this->m_Body;
  }
  const CTimeStamp& timeStamp() const {
    return this->m_TimeStamp;
  }
  CAttach* attachment() const {
    if (this->m_Attach) this->m_Attach->addRef();
    return this->m_Attach;
  }
  friend std::ostream& operator <<(std::ostream& os, const CMail& x);

  ~CMail() {
    if (this->m_Attach)
      this->m_Attach->release();
  }
private:
  CTimeStamp m_TimeStamp;
  std::string m_From;
  CMailBody m_Body;
  CAttach* m_Attach;
};

std::ostream& operator << (std::ostream& os, const CMail& x) {
  os << x.m_TimeStamp << ' ' << x.m_From << ' ' << x.m_Body;

  if (x.m_Attach) {
    os << " + " << *(x.m_Attach);
  }

  return os;
}
//=================================================================================================
class CMailBox
{
public:
  CMailBox() {
    this->m_Folders["inbox"] = std::vector<CMail>();
  }
  bool delivery(const CMail& mail) {
    if (m_Folders.contains("inbox")) {
      auto& inbox = m_Folders["inbox"];
      auto it = std::lower_bound(inbox.begin(), inbox.end(), mail, [](const CMail& a, const CMail& b) { return a < b; });
      inbox.insert(it, mail);
      // inbox.push_back(mail);
      return true;
    }
    return false;
  }
  bool newFolder(const std::string& folderName) {
    if (m_Folders.contains(folderName)) return false;
    else this->m_Folders[folderName] = std::vector<CMail>();
    return true;
  }
  bool moveMail(const std::string& fromFolder, const std::string& toFolder) {
    if ((m_Folders.contains(fromFolder) && m_Folders.contains(toFolder)) && fromFolder != toFolder) {
      auto& from = m_Folders[fromFolder];
      auto& to = m_Folders[toFolder];
      // to.merge(from);
      auto mid = to.size();
      to.insert(to.end(), std::make_move_iterator(from.begin()), std::make_move_iterator(from.end()));
      from.clear();
      std::inplace_merge(to.begin(), to.begin() + mid, to.end());
      return true;
    }
    else return false;
  }
  std::list<CMail> listMail(const std::string& folderName, const CTimeStamp& from, const CTimeStamp& to) const {
    std::list<CMail> res;
    if (!m_Folders.contains(folderName)) return res;

    auto& folder = m_Folders.at(folderName);

    auto it = std::lower_bound(folder.begin(), folder.end(), from, [](const CMail& a, const CTimeStamp& b) { return a.timeStamp() < b; });

    for (; it != folder.end(); it++) {
      if (it->timeStamp() > to) break;
      else res.push_back(*it);
    }
    return res;
  }
  std::set<std::string> listAddr(const CTimeStamp& from, const CTimeStamp& to) const {
    std::set<std::string> res;
    for (auto& folder : m_Folders) {
      auto it = std::lower_bound(folder.second.begin(), folder.second.end(), from, [](const CMail& a, const CTimeStamp& b) {
        return a.timeStamp() < b;
        });
      for (; it != folder.second.end(); it++) {
        if (it->timeStamp() > to) break;
        res.insert(it->from());
      }

    }

    return res;
  }

private:
  std::map<std::string, std::vector<CMail>> m_Folders;
};
//=================================================================================================
#ifndef __PROGTEST__
static std::string showMail(const std::list<CMail>& l)
{
  std::ostringstream oss;
  for (const auto& x : l)
    oss << x << std::endl;
  return oss.str();
}
static std::string showUsers(const std::set<std::string>& s)
{
  std::ostringstream oss;
  for (const auto& x : s)
    oss << x << std::endl;
  return oss.str();
}
int main()
{
  for (int i = 0; i < 200; i++)
  {
    CAttach* att;
    std::ostringstream oss;

    att = new CAttach(100);
    CMail testMail(CTimeStamp(2026, 1, 2, 12, 5, 0), "test@domain.cz", CMailBody(10, "test, test"), att);
    att->release();
    assert(testMail.timeStamp() == CTimeStamp(2026, 1, 2, 12, 5, 0));
    assert(testMail.from() == "test@domain.cz");
    att = testMail.attachment();
    oss << *att;
    att->release();
    assert(oss.str() == "attachment: 100 B");
    assert(showMail({ testMail }) == "2026-01-02 12:05:00 test@domain.cz mail body: 10 B + attachment: 100 B\n");

    CMailBox m0;
    assert(m0.delivery(CMail(CTimeStamp(2024, 3, 31, 15, 24, 13), "user1@fit.cvut.cz", CMailBody(14, "mail content 1"), nullptr)));
    assert(m0.delivery(CMail(CTimeStamp(2024, 3, 31, 15, 26, 23), "user2@fit.cvut.cz", CMailBody(22, "some different content"), nullptr)));
    att = new CAttach(200);
    assert(m0.delivery(CMail(CTimeStamp(2024, 3, 31, 11, 23, 43), "boss1@fit.cvut.cz", CMailBody(14, "urgent message"), att)));
    assert(m0.delivery(CMail(CTimeStamp(2024, 3, 31, 18, 52, 27), "user1@fit.cvut.cz", CMailBody(14, "mail content 2"), att)));
    att->release();
    att = new CAttach(97);
    assert(m0.delivery(CMail(CTimeStamp(2024, 3, 31, 16, 12, 48), "boss1@fit.cvut.cz", CMailBody(24, "even more urgent message"), att)));
    att->release();

    // std::cout << showMail(m0.listMail("inbox",
    //   CTimeStamp(2000, 1, 1, 0, 0, 0),
    //   CTimeStamp(2050, 12, 31, 23, 59, 59)));
    assert(showMail(m0.listMail("inbox",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n"
      "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n"
      "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n");
    assert(showMail(m0.listMail("inbox",
      CTimeStamp(2024, 3, 31, 15, 26, 23),
      CTimeStamp(2024, 3, 31, 16, 12, 48))) == "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n");
    assert(showUsers(m0.listAddr(CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "boss1@fit.cvut.cz\n"
      "user1@fit.cvut.cz\n"
      "user2@fit.cvut.cz\n");
    assert(showUsers(m0.listAddr(CTimeStamp(2024, 3, 31, 15, 26, 23),
      CTimeStamp(2024, 3, 31, 16, 12, 48))) == "boss1@fit.cvut.cz\n"
      "user2@fit.cvut.cz\n");

    CMailBox m1;
    assert(m1.newFolder("work"));
    assert(m1.newFolder("spam"));
    assert(!m1.newFolder("spam"));
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 15, 24, 13), "user1@fit.cvut.cz", CMailBody(14, "mail content 1"), nullptr)));
    att = new CAttach(500);
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 15, 26, 23), "user2@fit.cvut.cz", CMailBody(22, "some different content"), att)));
    att->release();
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 11, 23, 43), "boss1@fit.cvut.cz", CMailBody(14, "urgent message"), nullptr)));
    att = new CAttach(468);
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 18, 52, 27), "user1@fit.cvut.cz", CMailBody(14, "mail content 2"), att)));
    att->release();
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 16, 12, 48), "boss1@fit.cvut.cz", CMailBody(24, "even more urgent message"), nullptr)));
    assert(showMail(m1.listMail("inbox",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
      "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
    assert(showMail(m1.listMail("work",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
    assert(m1.moveMail("inbox", "work"));
    assert(showMail(m1.listMail("inbox",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
    assert(showMail(m1.listMail("work",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
      "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 19, 24, 13), "user2@fit.cvut.cz", CMailBody(14, "mail content 4"), nullptr)));
    att = new CAttach(234);
    assert(m1.delivery(CMail(CTimeStamp(2024, 3, 31, 13, 26, 23), "user3@fit.cvut.cz", CMailBody(9, "complains"), att)));
    att->release();
    assert(showMail(m1.listMail("inbox",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
      "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");
    assert(showMail(m1.listMail("work",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
      "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
    assert(m1.moveMail("inbox", "work"));
    assert(showMail(m1.listMail("inbox",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
    assert(showMail(m1.listMail("work",
      CTimeStamp(2000, 1, 1, 0, 0, 0),
      CTimeStamp(2050, 12, 31, 23, 59, 59))) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
      "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
      "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
      "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
      "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n"
      "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");

  }
  // --------------------------------------------------------------------------------------

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
