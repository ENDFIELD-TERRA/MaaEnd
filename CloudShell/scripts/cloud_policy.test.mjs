import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import test from "node:test";
import vm from "node:vm";

const policy = readFileSync(new URL("./cloud_policy.js", import.meta.url), "utf8");

function makeHarness() {
    let now = 1000;
    const listeners = new Map();
    class Document {}
    class HTMLCanvasElement {}
    class Element {}
    class MouseEvent {}
    Element.prototype.requestPointerLock = function () {
        return Promise.resolve("granted");
    };
    const window = {
        addEventListener(type, handler) {
            listeners.set(type, handler);
        },
    };
    const document = {
        getElementById() {
            return null;
        },
    };
    const context = {
        window,
        document,
        Document,
        HTMLCanvasElement,
        Element,
        MouseEvent,
        DOMException,
        performance: {
            now() {
                return now;
            },
            advance(ms) {
                now += ms;
            },
        },
    };
    vm.runInNewContext(policy, context, { filename: "cloud_policy.js" });
    return { window, listeners, Element, context };
}

function trustedEvent(fields = {}) {
    const event = {
        isTrusted: true,
        clientX: 0,
        clientY: 0,
        buttons: 0,
        cancelable: true,
        timeStamp: 1000,
        stopped: false,
        prevented: false,
        stopImmediatePropagation() {
            this.stopped = true;
        },
        preventDefault() {
            this.prevented = true;
        },
        ...fields,
    };
    return event;
}

test("physical movement keeps movement; automated positioning move is zeroed", () => {
    const { window, listeners } = makeHarness();
    const move = listeners.get("mousemove");

    const physical = trustedEvent({ clientX: 300, clientY: 400 });
    move(physical);
    assert.equal(Object.getOwnPropertyDescriptor(physical, "movementX"), undefined);

    window.__maaShellExpect(300, 400);
    const automated = trustedEvent({ clientX: 300, clientY: 400 });
    move(automated);
    assert.equal(automated.movementX, 0);
    assert.equal(automated.movementY, 0);
});

test("look movement preserves movement and zero-next zeroes the return move", () => {
    const { window, listeners } = makeHarness();
    const move = listeners.get("mousemove");

    window.__maaShellLook(100, 200);
    const look = trustedEvent({ clientX: 100, clientY: 200 });
    move(look);
    assert.equal(Object.getOwnPropertyDescriptor(look, "movementX"), undefined);

    window.__maaShellZeroNext();
    const back = trustedEvent({ clientX: 640, clientY: 360 });
    move(back);
    assert.equal(back.movementX, 0);
    assert.equal(back.movementY, 0);
});

test("physical mousedown is allowed; background physical move is dropped", () => {
    const { window, listeners } = makeHarness();
    const down = listeners.get("mousedown");
    const move = listeners.get("mousemove");

    const physical = trustedEvent({ clientX: 10, clientY: 20, buttons: 1 });
    down(physical);
    assert.equal(physical.stopped, false);
    assert.equal(physical.prevented, false);

    window.__maaShellSetForeground(false);
    const background = trustedEvent({ clientX: 30, clientY: 40 });
    move(background);
    assert.equal(background.stopped, true);
    assert.equal(background.prevented, true);
});

test("automated input without human activity is delivered", () => {
    const { window, listeners } = makeHarness();
    const down = listeners.get("mousedown");
    window.__maaShellExpect(10, 20);
    const automated = trustedEvent({ clientX: 10, clientY: 20, buttons: 1 });
    down(automated);
    assert.equal(automated.stopped, false);
});

test("pointer lock follows the most recent physical press", async () => {
    const { window, listeners, Element, context } = makeHarness();
    const element = new Element();
    const down = listeners.get("mousedown");

    window.__maaShellExpect(10, 20);
    await assert.rejects(element.requestPointerLock(), /after automated input/);

    context.performance.advance(10);
    down(trustedEvent({ clientX: 30, clientY: 40, buttons: 1 }));
    assert.equal(await element.requestPointerLock(), "granted");
});
