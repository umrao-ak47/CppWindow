# API Reference

The API reference is generated from Doxygen comments in
[`include/cppwindow/cppwindow.hpp`](../include/cppwindow/cppwindow.hpp).

Build it with Doxygen installed:

```bash
cmake -S . -B build-docs -DCPPWINDOW_BUILD_DOCS=ON -DCPPWINDOW_BUILD_EXAMPLES=OFF -DCPPWINDOW_BUILD_TESTS=OFF
cmake --build build-docs --target cppwindow_docs
```

Generated HTML is written to:

```text
build-docs/docs/html/index.html
```

Generated files are build artifacts and should not be committed.
