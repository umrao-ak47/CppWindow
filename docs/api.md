# API Reference

The API reference is generated from Doxygen comments in public headers under
[`include/cppwindow`](../include/cppwindow) and optional integration headers
under [`extras/imgui/include/cppwindow`](../extras/imgui/include/cppwindow).

Build it with Doxygen installed:

```bash
cmake --preset docs
cmake --build --preset docs
```

Generated HTML is written to:

```text
build-presets/docs/docs/html/index.html
```

Generated files are build artifacts and should not be committed.
