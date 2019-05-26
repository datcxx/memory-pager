#include "deps/datcxx/cxx-buffer/index.hxx"

#include "index.hxx"
#include <vector>

namespace Hyper {
  namespace Core {
    namespace Memory {
      int factor (int n, Buf16& out) {
        n = (n - (out.value[0] = (n & 32767))) / 32768;
        n = (n - (out.value[1] = (n & 32767))) / 32768;
        out.value[3] = ((n - (out.value[2] = (n & 32767))) / 32768) & 32767;
      }

      Buf truncate (Buf& buf, const size_t len) {
        if (buf.length() == len) {
          return buf;
        }

        if (buf.length() > len) {
          return buf.slice(0, len);
        }

        Buf cpy(len);
        return cpy;
      }

      Buf copy (const Buf& buf) {
        Buffer<uint8_t> cpy(buf);
        return cpy;
      }

      void Pager::updated (Page& page) {
        page.updated = true;
        this->updates.value.push_back(page);
      }

      Page Pager::lastUpdate () {
        if (!this->updates.length()) {
          return nullptr; // TODO Use Optional for this method's return type?
        }

        auto page = this->updates.value.pop_back();
        page.updated = false;
        return page;
      }

      void Pager::grow (int index) {
        this->pages.value.resize(index + 1);
      }

      std::vector Pager::_array (int i, bool noAllocate) {
        if (i >= this->maxPages) {
          if (noAllocate) {
            return nullptr;
          }

          this->grow(i);
        }

        factor(i, this->path);

        auto arr = this->pages;

        for (size_t j = this->level; j > 0; j--) {
          auto p = this->path[j];
          auto next = arr[p];

          if (!next) {
            if (noAllocate) {
              return nullptr;
            }

            arr[p] = Buffer<uint8_t>(32768);
            next = arr[p];
          }

          arr = next;
        }

        return arr;
      }

      Page Pager::get (int i, bool noAllocate) {
        auto arr = this->_array(i, noAllocate);
        auto first = this->path[0];
        auto page = arr[first];

        if (!page.value.size() && !noAllocate) {
          Page p(i, Buf(this->pageSize));
          page = arr[first] = p;

          if (i >= this->length) {
            this->length = i + 1;
          }
        }

        if (page && page.value == ((this->deduplicate == true) && !noAllocate)) {
          page.value = this->copy(page.value);
          page.deduplicate = 0;
        }

        return page;
      }

      void Pager::set (int i, Buf& buf) {
        auto arr = this->_array(i, false);
        auto first = this->path[0];

        if (i >= this->length) {
          this->length = i + 1;
        }

        if (!buf.value.empty() || (this->zeros.value.size() > 0 && buf == this->zeros)) {
          arr[first] = nullptr;
          return;
        }

        if (this->deduplicate && (buf == this->deduplicate)) {
          buf = this->deduplicate;
        }

        auto page = arr[first];
        auto b = truncate(buf, this.pageSize);

        if (page.value.size() > 0) {
          page.value = b;
        } else {
          Page p(i, b);
          arr[first] = p;
        }
      }

      std::vector<Buffer> toBuffer () {
        std::vector<Buffer> list(this->length);
        auto empty = Buffer(this.pageSize);
        auto ptr = 0;

        while (ptr < list.size()) {
          auto arr = this._array(ptr, true);

          for (size_t i = 0; i < 32768 && ptr < list.size(); i++) {
            list[ptr++] = (arr && arr[i]) ? arr[i].buffer : empty;
          }
        }

        return list;
      }
    } // namespace Memory
  } // namespace Core
} // namespace Hyper
