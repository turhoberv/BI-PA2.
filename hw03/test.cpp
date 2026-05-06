#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#endif /* __PROGTEST__ */

int min(int a, int b) {
  return (a < b) ? a : b;
}

struct Version {
  unsigned char* data;
  size_t size;
  size_t pos;

  Version() : data(nullptr), size(0), pos(0) {}

  Version(const char* src, size_t s, size_t p) : size(s), pos(p) {
    if (s > 0 && src) {
      data = new unsigned char[s];
      memcpy(data, src, s);
    }
    else data = nullptr;
  }

  void clear() {
    delete[] data;
    data = nullptr;
    size = 0;
    pos = 0;
  }
};

class CFileVector {
public:
  CFileVector() : sz(0), cap(10) {
    data = new Version[cap];
  }
  CFileVector(const CFileVector& other) : sz(0), cap(0), data(nullptr) {
    *this = other;
  }
  ~CFileVector() {
    for (int i = 0; i < sz; i++) data[i].clear();
    delete[] data;
  }
  CFileVector& operator=(const CFileVector& other) {
    if (this == &other) return *this;
    for (int i = 0; i < sz; i++) data[i].clear();
    delete[] data;

    sz = other.sz;
    cap = other.cap;
    data = new Version[cap];
    for (int i = 0; i < sz; i++) {
      data[i].size = other.data[i].size;
      data[i].pos = other.data[i].pos;
      if (other.data[i].data) {
        data[i].data = new unsigned char[data[i].size];
        memcpy(data[i].data, other.data[i].data, data[i].size);
      }
      else data[i].data = nullptr;
    }
    return *this;
  }
  void push_back(const Version& v) {
    if (sz >= cap) {
      cap = cap * 2 + 1;
      Version* newData = new Version[cap];
      for (int i = 0; i < sz; i++) {
        newData[i].data = data[i].data;
        newData[i].size = data[i].size;
        newData[i].pos = data[i].pos;
        data[i].data = nullptr;
      }
      delete[] data;
      data = newData;
    }
    data[sz].size = v.size;
    data[sz].pos = v.pos;
    if (v.size > 0 && v.data) {
      data[sz].data = new unsigned char[v.size];
      memcpy(data[sz].data, v.data, v.size);
    }
    else data[sz].data = nullptr;
    sz++;
  }
  Version& back() { return data[sz - 1]; }
  void pop_back() { if (sz > 0) { sz--; data[sz].clear(); } }
  int size() const { return sz; }
private:
  int sz;
  int cap;
  Version* data;
};

class CFile {
public:
  CFile() : size(0), pos(0), data(nullptr) {}
  CFile(const CFile& other) : data(nullptr) { *this = other; }
  ~CFile() { delete[] data; }

  CFile& operator=(const CFile& other) {
    if (this == &other) return *this;
    delete[] data;
    size = other.size;
    pos = other.pos;
    copies = other.copies;
    if (size > 0 && other.data) {
      data = new char[size];
      memcpy(data, other.data, size);
    }
    else data = nullptr;
    return *this;
  }

  bool seek(size_t offset) {
    if (offset <= size) { pos = (int)offset; return true; }
    return false;
  }

  size_t read(uint8_t dst[], size_t bytes) {
    if ((size_t)pos >= size) return 0;
    size_t canRead = min(bytes, size - pos);
    memcpy(dst, data + pos, canRead);
    pos += (int)canRead;
    return canRead;
  }

  size_t write(const uint8_t src[], size_t bytes) {
    size_t newSize = size;
    if ((size_t)pos + bytes > size) newSize = (size_t)pos + bytes;
    char* newData = new char[newSize];
    memset(newData, 0, newSize);
    if (data) memcpy(newData, data, size);
    memcpy(newData + pos, src, bytes);
    delete[] data;
    data = newData;
    size = newSize;
    pos += (int)bytes;
    return bytes;
  }

  void truncate() {
    size = (size_t)pos;
    if (size == 0) {
      delete[] data;
      data = nullptr;
      return;
    }
    char* newData = new char[size];
    if (data) memcpy(newData, data, size);
    delete[] data;
    data = newData;
  }

  size_t fileSize() const { return size; }

  void addVersion() {
    Version snapshot(this->data, this->size, (size_t)this->pos);
    copies.push_back(snapshot);
  }

  bool undoVersion() {
    if (copies.size() == 0) return false;
    Version& last = copies.back();
    delete[] data;
    size = last.size;
    data = (char*)last.data;
    pos = (int)last.pos;
    last.data = nullptr;
    copies.pop_back();
    return true;
  }

private:
  size_t size;
  int pos;
  char* data;
  CFileVector copies;
};





#ifndef __PROGTEST__
bool               writeTest(CFile& x, const std::initializer_list<uint8_t>& data, size_t            wrLen)
{
  return x.write(data.begin(), data.size()) == wrLen;
}

bool               readTest(CFile& x, const std::initializer_list<uint8_t>& data, size_t            rdLen)
{
  uint8_t  tmp[100];
  uint32_t idx = 0;

  if (x.read(tmp, rdLen) != data.size())
    return false;
  for (auto v : data)
    if (tmp[idx++] != v)
      return false;
  return true;
}





int main()
{


  CFile f0;

  CFile f2 = f0;

  assert(writeTest(f0, { 10, 20, 30 }, 3));
  assert(f0.fileSize() == 3);
  assert(writeTest(f0, { 60, 70, 80 }, 3));
  assert(f0.fileSize() == 6);
  assert(f0.seek(2));
  assert(writeTest(f0, { 5, 4 }, 2));
  assert(f0.fileSize() == 6);
  assert(f0.seek(1));
  assert(readTest(f0, { 20, 5, 4, 70, 80 }, 7));
  assert(f0.seek(3));
  f0.addVersion();
  assert(f0.seek(6));
  assert(writeTest(f0, { 100, 101, 102, 103 }, 4));
  f0.addVersion();
  assert(f0.seek(5));
  CFile f1(f0);
  f0.truncate();
  assert(f0.seek(0));
  assert(readTest(f0, { 10, 20, 5, 4, 70 }, 20));
  assert(f0.undoVersion());
  assert(f0.seek(0));
  assert(readTest(f0, { 10, 20, 5, 4, 70, 80, 100, 101, 102, 103 }, 20));
  assert(f0.undoVersion());
  assert(f0.seek(0));
  assert(readTest(f0, { 10, 20, 5, 4, 70, 80 }, 20));
  assert(!f0.seek(100));
  assert(writeTest(f1, { 200, 210, 220 }, 3));
  assert(f1.seek(0));
  assert(readTest(f1, { 10, 20, 5, 4, 70, 200, 210, 220, 102, 103 }, 20));
  assert(f1.undoVersion());
  assert(f1.undoVersion());
  assert(readTest(f1, { 4, 70, 80 }, 20));
  assert(!f1.undoVersion());

  f0.addVersion();
  f0.addVersion();
  f0.addVersion();
  f0.addVersion();
  f0.addVersion();
  f1 = f0;
  assert(f0.undoVersion());
  assert(f0.undoVersion());

  assert(f0.undoVersion());

  assert(f0.undoVersion());
  assert(f0.undoVersion());
  assert(f1.undoVersion());
  assert(f1.undoVersion());
  assert(f1.undoVersion());
  assert(f1.undoVersion());




  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
