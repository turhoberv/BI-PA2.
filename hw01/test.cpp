#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#endif /* __PROGTEST__ */


using namespace std;


string stringToLower(string s) { // used to compare case insensitive text
  int len = s.size();
  for (int i = 0; i < len; i++) {
    s[i] = tolower(s[i]);
  }
  return s;
}


class Company {
public:
  Company(string i, string n, string a) : name(n), address(a), id(i) {
    this->totalInvoices = 0;
    this->lowerAddress = stringToLower(a);
    this->lowerName = stringToLower(n);
  };
  string  getID() const {
    return this->id;
  }
  string   getName() const {
    return this->lowerName;
  }
  string  getAddress() const {
    return this->lowerAddress;
  }
  string   getOriginalName() const {
    return this->name;
  }
  string  getOriginalAddress() const {
    return this->address;
  }
  static bool cmpByID(const Company& a, const string& id) {
    return a.getID() < id;
  }
  static bool cmpByName(const Company& a, const Company& b) {
    return a.getName() < b.getName() || (a.getName() == b.getName() && a.getAddress() < b.getAddress());
  }
  bool addInvoice(int x) {
    return this->totalInvoices += x;
  }
  unsigned int getInvoices() const {
    return this->totalInvoices;
  }
private:
  string name;
  string address;
  string lowerName;
  string lowerAddress;
  string id;
  unsigned int totalInvoices;

};

class CVATRegister
{
public:



  CVATRegister() {};
  ~CVATRegister() {};
  bool newCompany(const std::string& name, const std::string& addr, const std::string& taxID) {
    string lowerName = stringToLower(name);
    string lowerAddr = stringToLower(addr);
    int byIDPos = CompaniesByID.size(), byNamePos = CompaniesByName.size();
    for (unsigned int i = 0; i < CompaniesByID.size(); i++) {
      if (CompaniesByID[i].getID() == taxID) { return false; }
      if (CompaniesByID[i].getID() > taxID) {
        byIDPos = i;
        break;
      }
    }
    for (unsigned int i = 0; i < CompaniesByName.size(); i++) {
      if (CompaniesByName[i].getName() == lowerName && CompaniesByName[i].getAddress() == lowerAddr) {
        return false;
      }
      if (CompaniesByName[i].getName() > lowerName || (CompaniesByName[i].getName() == lowerName && CompaniesByName[i].getAddress() > lowerAddr)) {
        byNamePos = i;
        break;
      }
    }
    CompaniesByID.insert(CompaniesByID.begin() + byIDPos, { taxID, name, addr });
    CompaniesByName.insert(CompaniesByName.begin() + byNamePos, { taxID, name, addr });
    return 1;
  }


  bool cancelCompany(const std::string& name, const std::string& addr) {
    Company searchPattern("", name, addr);

    string lowerName = stringToLower(name);
    string lowerAddress = stringToLower(addr);

    auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);

    if (Nameit == CompaniesByName.end() || (Nameit->getName() != lowerName || Nameit->getAddress() != lowerAddress)) {
      return false;
    }

    string id = Nameit->getID();

    auto IDit = lower_bound(CompaniesByID.begin(), CompaniesByID.end(), id, Company::cmpByID);

    CompaniesByID.erase(IDit);
    CompaniesByName.erase(Nameit);

    return 1;


  }


  bool cancelCompany(const std::string& taxID) {
    auto IDit = lower_bound(CompaniesByID.begin(), CompaniesByID.end(), taxID, Company::cmpByID);
    if (IDit == CompaniesByID.end() || IDit->getID() != taxID) {
      return false;
    }
    else {
      string name = IDit->getName();
      string addr = IDit->getAddress();

      Company searchPattern("", name, addr);

      auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);

      CompaniesByID.erase(IDit);
      CompaniesByName.erase(Nameit);
    }
    return 1;
  }


  bool invoice(const std::string& taxID, unsigned int           amount) {
    auto IDit = lower_bound(CompaniesByID.begin(), CompaniesByID.end(), taxID, Company::cmpByID);
    if (IDit == CompaniesByID.end() || IDit->getID() != taxID) {
      return false;
    }
    else {
      string name = IDit->getName();
      string addr = IDit->getAddress();

      Company searchPattern("", name, addr);

      auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);
      IDit->addInvoice(amount);
      Nameit->addInvoice(amount);
      this->invoices.push_back(amount);
      return true;
    }
  }


  bool invoice(const std::string& name, const std::string& addr, unsigned int           amount) {

    Company searchPattern("", name, addr);

    auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);

    string lowerName = stringToLower(name);
    string lowerAddress = stringToLower(addr);


    if (Nameit == CompaniesByName.end() || Nameit->getName() != lowerName || Nameit->getAddress() != lowerAddress) {
      return false;
    }

    string id = Nameit->getID();

    auto IDit = lower_bound(CompaniesByID.begin(), CompaniesByID.end(), id, Company::cmpByID);

    IDit->addInvoice(amount);
    Nameit->addInvoice(amount);
    this->invoices.push_back(amount);
    return true;

  }


  bool auditCompany(const std::string& name, const std::string& addr, unsigned int& sumIncome) const {

    Company searchPattern("", name, addr);


    string lowerName = stringToLower(name);
    string lowerAddress = stringToLower(addr);

    auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);

    if (Nameit == CompaniesByName.end() || Nameit->getName() != lowerName || Nameit->getAddress() != lowerAddress) {
      return false;
    }
    sumIncome = Nameit->getInvoices();
    return true;
  }


  bool auditCompany(const std::string& taxID, unsigned int& sumIncome) const {
    auto IDit = lower_bound(CompaniesByID.begin(), CompaniesByID.end(), taxID, Company::cmpByID);
    if (IDit == CompaniesByID.end() || IDit->getID() != taxID) {
      return false;
    }
    sumIncome = IDit->getInvoices();
    return true;
  }


  bool          firstCompany(std::string& name, std::string& addr) const {
    if (CompaniesByName.size() == 0) return false;
    name = CompaniesByName.at(0).getOriginalName();
    addr = CompaniesByName.at(0).getOriginalAddress();
    return true;
  }
  bool          nextCompany(std::string& name, std::string& addr) const {
    Company searchPattern("", name, addr);

    auto Nameit = lower_bound(CompaniesByName.begin(), CompaniesByName.end(), searchPattern, Company::cmpByName);

    string lowerName = stringToLower(name);
    string lowerAddress = stringToLower(addr);

    if (Nameit == CompaniesByName.end() || Nameit->getName() != lowerName || Nameit->getAddress() != lowerAddress) {
      return false;
    }
    auto Nextit = Nameit + 1;
    if (Nextit == CompaniesByName.end()) {
      return false;
    }
    name = Nextit->getOriginalName();
    addr = Nextit->getOriginalAddress();
    return true;

  }
  unsigned int  medianInvoice() const {
    vector<int> tmp = this->invoices;
    int size = tmp.size();
    if (size == 0) return 0;
    sort(tmp.begin(), tmp.end());
    int half = size / 2;
    return tmp.at(half);
  }
private:
  vector<Company> CompaniesByID;
  vector<int> invoices;




  vector<Company> CompaniesByName;
};

#ifndef __PROGTEST__
int               main()
{
  std::string name, addr;
  unsigned int sumIncome;

  CVATRegister b1;
  assert(b1.newCompany("ACME", "Thakurova", "666/666"));
  assert(b1.newCompany("ACME", "Kolejni", "666/666/666"));
  assert(b1.newCompany("Dummy", "Thakurova", "123456"));
  assert(b1.invoice("666/666", 2000));
  assert(b1.medianInvoice() == 2000);
  assert(b1.invoice("666/666/666", 3000));
  assert(b1.medianInvoice() == 3000);
  assert(b1.invoice("123456", 4000));
  assert(b1.medianInvoice() == 3000);
  assert(b1.invoice("aCmE", "Kolejni", 5000));
  assert(b1.medianInvoice() == 4000);
  assert(b1.auditCompany("ACME", "Kolejni", sumIncome) && sumIncome == 8000);
  assert(b1.auditCompany("123456", sumIncome) && sumIncome == 4000);
  assert(b1.firstCompany(name, addr) && name == "ACME" && addr == "Kolejni");
  assert(b1.nextCompany(name, addr) && name == "ACME" && addr == "Thakurova");
  assert(b1.nextCompany(name, addr) && name == "Dummy" && addr == "Thakurova");
  assert(!b1.nextCompany(name, addr));
  assert(b1.cancelCompany("ACME", "KoLeJnI"));
  assert(b1.medianInvoice() == 4000);
  assert(b1.cancelCompany("666/666"));
  assert(b1.medianInvoice() == 4000);
  assert(b1.invoice("123456", 100));
  assert(b1.medianInvoice() == 3000);
  assert(b1.invoice("123456", 300));
  assert(b1.medianInvoice() == 3000);
  assert(b1.invoice("123456", 200));
  assert(b1.medianInvoice() == 2000);
  assert(b1.invoice("123456", 230));
  assert(b1.medianInvoice() == 2000);
  assert(b1.invoice("123456", 830));
  assert(b1.medianInvoice() == 830);
  assert(b1.invoice("123456", 1830));
  assert(b1.medianInvoice() == 1830);
  assert(b1.invoice("123456", 2830));
  assert(b1.medianInvoice() == 1830);
  assert(b1.invoice("123456", 2830));
  assert(b1.medianInvoice() == 2000);
  assert(b1.invoice("123456", 3200));
  assert(b1.medianInvoice() == 2000);
  assert(b1.firstCompany(name, addr) && name == "Dummy" && addr == "Thakurova");
  assert(!b1.nextCompany(name, addr));
  assert(b1.cancelCompany("123456"));
  assert(!b1.firstCompany(name, addr));

  CVATRegister b2;
  assert(b2.newCompany("ACME", "Kolejni", "abcdef"));
  assert(b2.newCompany("Dummy", "Kolejni", "123456"));
  assert(!b2.newCompany("AcMe", "kOlEjNi", "1234"));
  assert(b2.newCompany("Dummy", "Thakurova", "ABCDEF"));
  assert(b2.medianInvoice() == 0);
  assert(b2.invoice("ABCDEF", 1000));
  assert(b2.medianInvoice() == 1000);
  assert(b2.invoice("abcdef", 2000));
  assert(b2.medianInvoice() == 2000);
  assert(b2.invoice("aCMe", "kOlEjNi", 3000));
  assert(b2.medianInvoice() == 2000);
  assert(!b2.invoice("1234567", 100));
  assert(!b2.invoice("ACE", "Kolejni", 100));
  assert(!b2.invoice("ACME", "Thakurova", 100));
  assert(!b2.auditCompany("1234567", sumIncome));
  assert(!b2.auditCompany("ACE", "Kolejni", sumIncome));
  assert(!b2.auditCompany("ACME", "Thakurova", sumIncome));
  assert(!b2.cancelCompany("1234567"));
  assert(!b2.cancelCompany("ACE", "Kolejni"));
  assert(!b2.cancelCompany("ACME", "Thakurova"));
  assert(b2.cancelCompany("abcdef"));
  assert(b2.medianInvoice() == 2000);
  assert(!b2.cancelCompany("abcdef"));
  assert(b2.newCompany("ACME", "Kolejni", "abcdef"));
  assert(b2.cancelCompany("ACME", "Kolejni"));
  assert(!b2.cancelCompany("ACME", "Kolejni"));

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
