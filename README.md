# kvik
## About
**kvik** is key-value in-memory storage with support of strings, lists, sets, geospatial-indexes and TTL.

## Development notes
1. Use clang-format not to think about code style (all git staged files wth listed extensions will be formated):
```bash
clang-format -i $(git ls-files '*.cpp' '*.hpp' '*.h')
```