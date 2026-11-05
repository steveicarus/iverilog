/* Inspect both enum metadata and value changes, independently of the dumper. */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "fstapi.h"

static void value_change(void*data, uint64_t time, fstHandle handle,
                         const unsigned char*value)
{
      (void)data;
      printf("VALUE %" PRIu64 " %u %s\n", time, handle, value);
}

int main(int argc, char**argv)
{
      assert(argc == 2);
      fstReaderContext*reader = fstReaderOpen(argv[1]);
      assert(reader);
      struct fstHier*item;
      fstEnumHandle pending = 0;
      while ((item = fstReaderIterateHier(reader))) {
            if (item->htyp == FST_HT_SCOPE) {
                  fstReaderPushScope(reader, item->u.scope.name, NULL);
            } else if (item->htyp == FST_HT_UPSCOPE) {
                  fstReaderPopScope(reader);
            } else if (item->htyp == FST_HT_ATTRBEGIN &&
                item->u.attr.typ == FST_AT_MISC &&
                item->u.attr.subtype == FST_MT_ENUMTABLE) {
                  pending = item->u.attr.arg;
                  if (item->u.attr.name && item->u.attr.name[0]) {
                        struct fstETab*table = fstUtilityExtractEnumTableFromString(item->u.attr.name);
                        assert(table);
                        for (unsigned n = 0; n < table->elem_count; n += 1)
                              printf("TABLE %u %s %s\n", pending,
                                     table->literal_arr[n], table->val_arr[n]);
                        fstUtilityFreeEnumTable(table);
                  }
            } else if (item->htyp == FST_HT_VAR) {
                  printf("VAR %u %u %s.%s\n", item->u.var.handle, pending,
                         fstReaderGetCurrentFlatScope(reader), item->u.var.name);
                  pending = 0;
            }
      }
      fstReaderSetFacProcessMaskAll(reader);
      assert(fstReaderIterBlocks(reader, value_change, NULL, NULL));
      fstReaderClose(reader);
      return 0;
}
