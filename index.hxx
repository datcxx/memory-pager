#ifndef HYPER_CORE_MEMORY_H
#define HYPER_CORE_MEMORY_H

#include <vector>

#include "deps/datcxx/cxx-buffer/index.hxx"

namespace Hyper {
  namespace Core {
    namespace Memory {
      using Buf = Buffer<uint8_t>;
      using Buf16 = Buffer<uint16_t>;

      struct Page {
        size_t deduplicate = 0;
        size_t offset = 0;
        bool updated = false;
        Buf buffer;

        Page (int i, Buf buf) {
          this->offset = i * buf.length();
          this->buffer.value = buf.value;
        }
      };

      struct PageOptions {
        Buffer<Page> deduplicate;
      };

      int factor (int n, Buf16& out);
      Buf truncate (Buf& buf, const size_t len);
      Buf copy (const Buf& buf);

      class Pager {
        public:
          Buffer<uint16_t> path {4};
          Buffer<uint8_t> zeros;
          Page lastUpdate();
          Page get(int i, bool noAllocate);

          void updated(Page& page);
          void grow(int index);

          Buffer<Page> pages;
          Buffer<Page> updates;
          Buffer<Page> deduplicate;
          std::vector<Buf> _array(int i, bool noAllocate);
          std::vector<Buf> toBuffer();
          void set (int i, Buf& buf);

          size_t length = 0;
          size_t pageSize = 1024;
          size_t maxPages = pages.length();
          size_t level = 0;

          PageOptions opts;

          Pager (int pageSize, PageOptions opts = {}) :
            pageSize(pageSize),
            opts(opts) {
              zeros.value.resize(deduplicate.length());
          };
      };
    } // namespace Memory
  } // namespace Core
} // namespace Hyper

#endif
