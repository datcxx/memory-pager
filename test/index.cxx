#include "../deps/heapwolf/cxx-tap/index.hxx"
#include "../deps/datcxx/cxx-buffer/index.hxx"

#include "../index.hxx"

#include <iostream>
#include <sstream>
#include <limits>

int main() {
  TAP::Test t;
  namespace Memory = Hyper::Core::Memory;

  t.test("sanity", [](auto t) {
    t->ok(true, "true is true");
    t->end();
  });

  t.test("get page", [](auto t) {
    auto pages = Memory::Pager(1024);

    auto page = pages.get(0);

    t->equal(page.offset, 0);
    t->equal(page.buffer, Buffer<uint8_t>(1024));
    t->end();
  });

  t.test("get page twice", [](auto t) {
    auto pages = Memory::Pager(1024);
    t->equal(pages.length, 0);

    auto page = pages.get(0);

    t->equal(page.offset, 0);
    t->equal(page.buffer, Buffer<uint8_t>(1024));
    t->equal(pages.length, 1);

    auto other = pages.get(0);

    t->equal(other, page);
    t->end();
  });

  t.test("get no mutable page", [](auto t) {
    auto pages = Memory::Pager(1024);

    t->ok(!pages.get(141, true));
    t->ok(pages.get(141));
    t->ok(pages.get(141, true));

    t->end();
  });

  t.test("get far out page", [](auto t) {
    auto pages = Memory::Pager(1024);

    auto page = pages.get(1000000);

    t->equal(page.offset, 1000000 * 1024);
    t->equal(page.buffer, Buffer<uint8_t>(1024));
    t->equal(pages.length, 1000000 + 1);

    auto other = pages.get(1);

    t->equal(other.offset, 1024);
    t->equal(other.buffer, Buffer<uint8_t>(1024));
    t->equal(pages.length, 1000000 + 1);
    t->ok(other != page);

    t->end();
  });

  t.test("updates", [](auto t) {
    auto pages = Memory::Pager(1024);

    t->equal(pages.lastUpdate(), nullptr);

    auto page = pages.get(10);

    page->buffer[42] = 1;
    pages->updated(page);

    t->equal(pages.lastUpdate(), page);
    t->equal(pages.lastUpdate(), nullptr);

    page->buffer[42] = 2;
    pages->updated(page);
    pages->updated(page);

    t->equal(pages.lastUpdate(), page);
    t->equal(pages.lastUpdate(), nullptr);

    t->end();
  });
}
