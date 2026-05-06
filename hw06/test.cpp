#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ipaddress.h"
using namespace std::literals;
#endif /* __PROGTEST__ */


class CRec {
public:
    CRec(const std::string &name) : m_DomainName(name) {}
    std::string name() const { return m_DomainName; }
    virtual std::string type() const = 0;
    virtual void print(std::ostream &os) const = 0;
    friend std::ostream &operator<<(std::ostream &os, const CRec &crc) {
        crc.print(os);
        return os;
    }
    virtual CRec *copy() const = 0;
    virtual bool isEqual(const CRec &other) const {
        return m_DomainName == other.m_DomainName && type() == other.type();
    }
    virtual ~CRec() = default;


protected:
    std::string m_DomainName;
};

class CRecA : public CRec {
public:
    // constructor
    CRecA(const std::string &name, CIPv4 ipv4) : CRec(name), m_IPv4(ipv4) {}
    // name () -- inherited
    // type ()
    std::string type() const override { return "A"; }
    CIPv4 IPv4() const { return m_IPv4; }
    // operator << - inherited
    void print(std::ostream &os) const override { os << m_DomainName << ' ' << type() << ' ' << m_IPv4; }
    virtual bool isEqual(const CRec &other) const override {
        return CRec::isEqual(other) && m_IPv4 == dynamic_cast<const CRecA &>(other).IPv4();
    }
    virtual CRec *copy() const override { return new CRecA(m_DomainName, m_IPv4); };


private:
    CIPv4 m_IPv4;
};

class CRecAAAA : public CRec {
public:
    // constructor
    CRecAAAA(const std::string &name, CIPv6 ipv6) : CRec(name), m_IPv6(ipv6) {}
    // name () -- inherited
    // type ()
    std::string type() const override { return "AAAA"; }
    // operator <<
    void print(std::ostream &os) const override { os << m_DomainName << ' ' << type() << ' ' << m_IPv6; }
    CIPv6 IPv6() const { return m_IPv6; }
    virtual bool isEqual(const CRec &other) const override {
        return CRec::isEqual(other) && m_IPv6 == dynamic_cast<const CRecAAAA &>(other).IPv6();
    }
    virtual CRec *copy() const override { return new CRecAAAA(m_DomainName, m_IPv6); }

private:
    CIPv6 m_IPv6;
};

class CRecMX : public CRec {
public:
    // constructor
    CRecMX(const std::string &domainName, const std::string &mailName, int priority) :
        CRec(domainName), m_priority(priority), m_MailName(mailName) {}
    // name () -- inherited
    // type ()
    std::string type() const override { return "MX"; }
    int priority() const { return m_priority; }
    std::string mailName() const { return m_MailName; }
    // operator <<
    void print(std::ostream &os) const override {
        os << m_DomainName << ' ' << type() << ' ' << m_priority << ' ' << m_MailName;
    }
    virtual bool isEqual(const CRec &other) const override {
        return CRec::isEqual(other) && m_priority == dynamic_cast<const CRecMX &>(other).priority() &&
               m_MailName == dynamic_cast<const CRecMX &>(other).mailName();
    }

    virtual CRec *copy() const override { return new CRecMX(m_DomainName, m_MailName, m_priority); }


private:
    int m_priority;
    std::string m_MailName;
};

class RecordSet {
public:
    RecordSet() = default;

    friend std::ostream &operator<<(std::ostream &os, const RecordSet &rec) {
        for (auto &record: rec.m_Records) {
            os << *record << std::endl;
        }
        return os;
    }
    void push_back(const CRec &rec) { m_Records.push_back(std::unique_ptr<CRec>(rec.copy())); }
    const CRec &operator[](const size_t pos) const {
        if (pos >= m_Records.size()) {
            throw std::out_of_range(std::to_string(pos));
        }
        return *m_Records[pos];
    }
    size_t size() const { return m_Records.size(); }

private:
    std::vector<std::unique_ptr<CRec>> m_Records;
};

class CZone {
public:
    // constructor(s)
    CZone(const std::string &name) : m_ZoneName(name) {}

    CZone(const CZone &other) {
        m_ZoneName = other.m_ZoneName;
        for (const auto &record: other.m_Records) {
            m_Records.push_back(std::unique_ptr<CRec>(record->copy()));
        }
    }

    CZone &operator=(const CZone &other) {
        if (this == &other)
            return *this;
        m_ZoneName = other.m_ZoneName;
        m_Records.clear();
        for (const auto &record: other.m_Records) {
            m_Records.push_back(std::unique_ptr<CRec>(record->copy()));
        }
        return *this;
    }

    // destructor (if needed)
    ~CZone() = default;
    // operator = (if needed)
    // add ()
    bool add(const CRec &rec) {
        for (auto &record: m_Records) {
            if (record->isEqual(rec)) {
                return false;
            }
        }
        m_Records.push_back(std::unique_ptr<CRec>(rec.copy()));
        return true;
    }
    // del ()
    bool del(const CRec &rec) {
        for (auto it = m_Records.begin(); it != m_Records.end(); ++it) {
            if ((*it)->isEqual(rec)) {
                m_Records.erase(it);
                return true;
            }
        }
        return false;
    }
    // search ()
    RecordSet search(const std::string &searchedName) const {
        RecordSet result;
        for (const auto &record: m_Records) {
            if (record->name() == searchedName) {
                result.push_back(*record);
            }
        }
        return result;
    }
    // operator <<
    friend std::ostream &operator<<(std::ostream &os, const CZone &zone) {
        os << zone.m_ZoneName << std::endl;
        for (auto it = zone.m_Records.begin(); it != zone.m_Records.end(); ++it) {
            if (std::next(it) == zone.m_Records.end()) {
                os << " \\- " << **it << std::endl;
            } else {
                os << " +- " << **it << std::endl;
            }
        }
        return os;
    }
    // todo
private:
    std::string m_ZoneName;
    std::list<std::unique_ptr<CRec>> m_Records;
};

#ifndef __PROGTEST__
int main() {
    std::ostringstream oss;

    CZone z0("fit.cvut.cz");
    assert(z0.add(CRecA("progtest", CIPv4("147.32.232.142"))) == true);
    assert(z0.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:0:1:2:3"))) == true);
    assert(z0.add(CRecA("courses", CIPv4("147.32.232.158"))) == true);
    assert(z0.add(CRecA("courses", CIPv4("147.32.232.160"))) == true);
    assert(z0.add(CRecA("courses", CIPv4("147.32.232.159"))) == true);
    assert(z0.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:1:2:3:4"))) == true);
    assert(z0.add(CRecMX("courses", "relay.fit.cvut.cz.", 0)) == true);
    assert(z0.add(CRecMX("courses", "relay2.fit.cvut.cz.", 10)) == true);
    oss.str("");

    oss << z0;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.160\n"
                        " +- courses A 147.32.232.159\n"
                        " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
                        " +- courses MX 0 relay.fit.cvut.cz.\n"
                        " \\- courses MX 10 relay2.fit.cvut.cz.\n");
    assert(z0.search("progtest").size() == 3);
    oss.str("");
    oss << z0.search("progtest");


    assert(oss.str() == "progtest A 147.32.232.142\n"
                        "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        "progtest AAAA 2001:718:2:2902:1:2:3:4\n");
    assert(z0.del(CRecA("courses", CIPv4("147.32.232.160"))) == true);
    assert(z0.add(CRecA("courses", CIPv4("147.32.232.122"))) == true);
    oss.str("");
    oss << z0;

    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.159\n"
                        " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
                        " +- courses MX 0 relay.fit.cvut.cz.\n"
                        " +- courses MX 10 relay2.fit.cvut.cz.\n"
                        " \\- courses A 147.32.232.122\n");
    assert(z0.search("courses").size() == 5);
    oss.str("");
    oss << z0.search("courses");
    assert(oss.str() == "courses A 147.32.232.158\n"
                        "courses A 147.32.232.159\n"
                        "courses MX 0 relay.fit.cvut.cz.\n"
                        "courses MX 10 relay2.fit.cvut.cz.\n"
                        "courses A 147.32.232.122\n");
    oss.str("");
    oss << z0.search("courses")[0];
    assert(oss.str() == "courses A 147.32.232.158");
    assert(z0.search("courses")[0].name() == "courses");
    assert(z0.search("courses")[0].type() == "A");
    oss.str("");
    oss << z0.search("courses")[1];
    assert(oss.str() == "courses A 147.32.232.159");
    assert(z0.search("courses")[1].name() == "courses");
    assert(z0.search("courses")[1].type() == "A");
    oss.str("");
    oss << z0.search("courses")[2];
    assert(oss.str() == "courses MX 0 relay.fit.cvut.cz.");
    assert(z0.search("courses")[2].name() == "courses");
    assert(z0.search("courses")[2].type() == "MX");
    try {
        oss.str("");
        oss << z0.search("courses")[10];
        assert("No exception thrown!" == nullptr);
    } catch (const std::out_of_range &e) {
    } catch (...) {
        assert("Invalid exception thrown!" == nullptr);
    }
    dynamic_cast<const CRecAAAA &>(z0.search("progtest")[1]);
    CZone z1("fit2.cvut.cz");
    z1.add(z0.search("progtest")[2]);
    z1.add(z0.search("progtest")[0]);
    z1.add(z0.search("progtest")[1]);
    z1.add(z0.search("courses")[2]);
    oss.str("");
    oss << z1;
    assert(oss.str() == "fit2.cvut.cz\n"
                        " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " \\- courses MX 0 relay.fit.cvut.cz.\n");
    dynamic_cast<const CRecA &>(z1.search("progtest")[1]);

    CZone z2("fit.cvut.cz");
    assert(z2.add(CRecA("progtest", CIPv4("147.32.232.142"))) == true);
    assert(z2.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:0:1:2:3"))) == true);
    assert(z2.add(CRecA("progtest", CIPv4("147.32.232.144"))) == true);
    assert(z2.add(CRecMX("progtest", "relay.fit.cvut.cz.", 10)) == true);
    assert(z2.add(CRecA("progtest", CIPv4("147.32.232.142"))) == false);
    assert(z2.del(CRecA("progtest", CIPv4("147.32.232.140"))) == false);
    assert(z2.del(CRecA("progtest", CIPv4("147.32.232.142"))) == true);
    assert(z2.del(CRecA("progtest", CIPv4("147.32.232.142"))) == false);
    assert(z2.add(CRecMX("progtest", "relay.fit.cvut.cz.", 20)) == true);
    assert(z2.add(CRecMX("progtest", "relay.fit.cvut.cz.", 10)) == false);
    oss.str("");
    oss << z2;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " +- progtest A 147.32.232.144\n"
                        " +- progtest MX 10 relay.fit.cvut.cz.\n"
                        " \\- progtest MX 20 relay.fit.cvut.cz.\n");
    assert(z2.search("progtest").size() == 4);
    oss.str("");
    oss << z2.search("progtest");
    assert(oss.str() == "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        "progtest A 147.32.232.144\n"
                        "progtest MX 10 relay.fit.cvut.cz.\n"
                        "progtest MX 20 relay.fit.cvut.cz.\n");
    assert(z2.search("courses").size() == 0);
    oss.str("");
    oss << z2.search("courses");
    assert(oss.str() == "");
    try {
        dynamic_cast<const CRecMX &>(z2.search("progtest")[0]);
        assert("Invalid type" == nullptr);
    } catch (const std::bad_cast &e) {
    }

    CZone z4("fit.cvut.cz");
    assert(z4.add(CRecA("progtest", CIPv4("147.32.232.142"))) == true);
    assert(z4.add(CRecA("courses", CIPv4("147.32.232.158"))) == true);
    assert(z4.add(CRecA("courses", CIPv4("147.32.232.160"))) == true);
    assert(z4.add(CRecA("courses", CIPv4("147.32.232.159"))) == true);
    CZone z5(z4);
    assert(z4.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:0:1:2:3"))) == true);
    assert(z4.add(CRecAAAA("progtest", CIPv6("2001:718:2:2902:1:2:3:4"))) == true);
    assert(z5.del(CRecA("progtest", CIPv4("147.32.232.142"))) == true);
    oss.str("");
    oss << z4;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.160\n"
                        " +- courses A 147.32.232.159\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " \\- progtest AAAA 2001:718:2:2902:1:2:3:4\n");
    oss.str("");

    oss << z5;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.160\n"
                        " \\- courses A 147.32.232.159\n");
    z5 = z4;
    assert(z4.add(CRecMX("courses", "relay.fit.cvut.cz.", 0)) == true);
    assert(z4.add(CRecMX("courses", "relay2.fit.cvut.cz.", 10)) == true);
    oss.str("");
    oss << z4;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.160\n"
                        " +- courses A 147.32.232.159\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
                        " +- courses MX 0 relay.fit.cvut.cz.\n"
                        " \\- courses MX 10 relay2.fit.cvut.cz.\n");
    oss.str("");
    oss << z5;
    assert(oss.str() == "fit.cvut.cz\n"
                        " +- progtest A 147.32.232.142\n"
                        " +- courses A 147.32.232.158\n"
                        " +- courses A 147.32.232.160\n"
                        " +- courses A 147.32.232.159\n"
                        " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
                        " \\- progtest AAAA 2001:718:2:2902:1:2:3:4\n");

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
