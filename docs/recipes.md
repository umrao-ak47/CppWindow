# CppWindow Recipes

These recipes show the intended shape of common app code. CppWindow owns
windows, input snapshots, event collection, and platform services. Your app
still owns the main loop.

## Game Loop

Use `pollEvents()` once per frame, dispatch events after polling, then update
and render from the current input snapshot.

```cpp
auto& ctx = cwin::WindowContext::get();
auto window = cwin::WindowBuilder{}
                  .title("Game")
                  .size(1280, 720)
                  .openGL({ 4, 1, true })
                  .resizable()
                  .build();

window.makeContextCurrent();
window.setVSync(true);

cwin::EventDispatcher dispatcher;
dispatcher.on<cwin::Event::Closed>([&] {
    window.requestClose();
});

cwin::FrameTimer timer;
cwin::FpsCounter fps;
cwin::FrameLimiter limiter(120.0);
limiter.setVSyncEnabled(true);

while (!window.shouldClose()) {
    cwin::FrameTime frame = timer.tick();

    ctx.pollEvents();
    dispatcher.dispatch(window.events());

    updateGame(frame.deltaSeconds, window.getInput());
    renderGame(window);
    window.swapBuffers();

    if (fps.update(frame)) {
        showFps(fps.framesPerSecond());
    }

    limiter.wait();
}
```

## Fixed-Step Simulation

Use `FixedStepAccumulator` for deterministic simulation and keep rendering
under caller control.

```cpp
cwin::FrameTimer timer;
cwin::FixedStepAccumulator fixedStep(1.0 / 60.0);

while (!window.shouldClose()) {
    ctx.pollEvents();

    const cwin::FrameTime frame = timer.tick();
    fixedStep.add(std::min(frame.deltaSeconds, 0.25));

    while (fixedStep.consumeStep()) {
        simulate(fixedStep.fixedDeltaSeconds());
    }

    renderInterpolated(fixedStep.alpha());
}
```

See `examples/fixed_step_loop.cpp` for a runnable version.

## Editor Or Tool Loop

For mostly-idle tools, use `waitEventsTimeout()` so the app sleeps until input
arrives but still wakes for autosave, animations, or background polling.

```cpp
auto& ctx = cwin::WindowContext::get();
auto window = cwin::WindowBuilder{}
                  .title("Tool")
                  .size(1200, 800)
                  .noGraphicsApi()
                  .resizable()
                  .build();

cwin::EventDispatcher dispatcher;
dispatcher
    .on<cwin::Event::Closed>([&] {
        window.requestClose();
    })
    .on<cwin::Event::FilesDropped>([&](const cwin::Event::FilesDropped& drop) {
        for (const std::string& path : drop.paths) {
            openDocument(path);
        }
    });

while (!window.shouldClose()) {
    ctx.waitEventsTimeout(0.25);
    dispatcher.dispatch(window.events());

    if (needsBackgroundTick()) {
        runBackgroundWork();
    }
}
```

Call `WindowContext::postEmptyEvent()` from another thread when it needs to
wake a waiting UI loop.

## Action Contexts

Use contexts to disable whole sets of actions, such as gameplay input while a
menu or text field is focused.

```cpp
cwin::ActionMap actions;
const cwin::ActionId jump = actions.getOrCreateActionId("jump");
const cwin::ActionId fire = actions.getOrCreateActionId("fire");
const cwin::ActionId moveX = actions.getOrCreateActionId("move_x");
const cwin::ActionId saveAction = actions.getOrCreateActionId("save");
const cwin::ActionId leftDash = actions.getOrCreateActionId("left_dash");

actions.bindKey(jump, cwin::Key::Space)
       .bindMouseButton(fire, cwin::MouseButton::Left)
       .bindGamepadButton(jump, cwin::GamepadButton::A)
       .bindGamepadAxis(moveX, cwin::GamepadAxis::LeftX, 0.20f)
       .bindKey(saveAction, cwin::Key::S, cwin::Modifiers{ .control = true })
       .bindKeyCombo(leftDash, cwin::Key::A, { cwin::Key::LShift })
       .setContext(jump, "gameplay")
       .setContext(fire, "gameplay")
       .setContext(moveX, "gameplay")
       .setContext(leftDash, "gameplay");

while (!window.shouldClose()) {
    ctx.pollEvents();

    actions.setContextEnabled("gameplay", !menuOpen);
    actions.update(window.getInput(), ctx.getGamepadState(0));

    if (actions.isPressed(saveAction)) {
        saveProject();
    }

    if (actions.isPressed(jump)) {
        jump();
    }

    movePlayer(actions.getAxis(moveX));
}
```

Use `replaceKey()`, `replaceKeyCombo()`, `replaceMouseButton()`,
`replaceGamepadButton()`, and `replaceGamepadAxis()` for rebinding screens.

## Multi-Window Routing

Each window has its own event queue and input snapshot. Keep one dispatcher per
window when behavior differs by window.

```cpp
auto inspector = cwin::WindowBuilder{}
                     .title("Inspector")
                     .size(520, 640)
                     .noGraphicsApi()
                     .resizable()
                     .build();

auto viewport = cwin::WindowBuilder{}
                    .title("Viewport")
                    .size(1280, 720)
                    .openGL({ 4, 1, true })
                    .resizable()
                    .build();

cwin::EventDispatcher inspectorEvents;
cwin::EventDispatcher viewportEvents;

inspectorEvents.on<cwin::Event::Closed>([&] {
    inspector.requestClose();
});

viewportEvents
    .on<cwin::Event::Closed>([&] {
        viewport.requestClose();
    })
    .on<cwin::Event::FramebufferResized>([&](const cwin::Event::FramebufferResized& size) {
        resizeViewport(size.width, size.height);
    });

while (!inspector.shouldClose() || !viewport.shouldClose()) {
    ctx.pollEvents();
    inspectorEvents.dispatch(inspector.events());
    viewportEvents.dispatch(viewport.events());
}
```

See `examples/multi_window.cpp` for a runnable version.

## Fullscreen Mode Selection

Use the mode that matches the user-visible behavior you want:

- `WindowMode::Fullscreen`: decorated maximized window with title bar.
- `WindowMode::BorderlessFullscreen`: undecorated window covering the monitor
  work area. Prefer this for most games and apps.
- `WindowMode::ExclusiveFullscreen`: true monitor fullscreen. This may change
  display mode, framebuffer size, content scale, and OS fullscreen behavior.

```cpp
dispatcher
    .on<cwin::Event::KeyPressed>([&](const cwin::Event::KeyPressed& key) {
        if (key.key == cwin::Key::F) {
            window.setWindowMode(cwin::WindowMode::Fullscreen);
        } else if (key.key == cwin::Key::B) {
            window.setWindowMode(cwin::WindowMode::BorderlessFullscreen);
        } else if (key.key == cwin::Key::E) {
            window.setWindowMode(cwin::WindowMode::ExclusiveFullscreen);
        } else if (key.key == cwin::Key::W) {
            window.setWindowMode(cwin::WindowMode::Windowed);
        }
    });
```

Renderers should size viewports and swapchains from `getFramebufferSize()` and
handle `Event::FramebufferResized`. Use `getDpiScale()` when converting between
window coordinates and framebuffer pixels.
