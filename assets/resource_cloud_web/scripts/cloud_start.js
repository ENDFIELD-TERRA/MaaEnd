// MaaEnd CloudShell 注入脚本：登录后自动点击「开始游戏」启动云游戏。
// 页面「开始游戏」按钮被透明 DIV 覆盖，Win32 点击落在覆盖层上无效，只能对底层 button 元素执行 JS click。
// 纯浏览器 IIFE，无依赖；由 cloud-shell.exe --inject 加载。
(() => {
    "use strict";

    if (window.__maaCloudStart && window.__maaCloudStart.running) {
        return;
    }
    if (window.__maaCloudStart && window.__maaCloudStart.timer) {
        clearInterval(window.__maaCloudStart.timer);
    }
    window.__maaCloudStart = { running: true, timer: 0 };

    const POLL_INTERVAL_MS = 1000;
    const CLICK_COOLDOWN_MS = 5000;
    const MAX_RUNTIME_MS = 30 * 60 * 1000;
    // 「游戏启动中」等文案在云端视频流里，页面 DOM 看不到；点过开始后不能无限重试，
    // 否则每次 DOM click 都可能把正在供给的云会话打回起点。
    const MAX_START_CLICKS = 2;
    const START_RETRY_MS = 20000;

    // 登录态标记：页面文本包含其一即视为已登录
    const LOGIN_MARKERS = ["通行证UID", "用户中心"];
    // 上次数据弹窗的确认按钮文案
    const ACK_TEXT = "知道了";
    // 落地页开始按钮文案
    const START_TEXT = "开始游戏";
    // SDK 快速登录按钮文案（只点精确的「登录」，不点「登录其他账号」）
    const LOGIN_TEXT = "登录";
    // 启动中/排队/连接异常时只等待，不重复点击开始
    const BUSY_TEXTS = ["游戏启动中", "请耐心等待", "正在排队", "连接服务器超时", "数据正在处理中"];

    const startedAt = Date.now();
    let lastClickAt = 0;
    let startClicks = 0;
    let lastStartAt = 0;
    let timer = 0;
    let ballTimer = 0;

    // CloudShell 左侧浮球是左上角的小型 fixed 容器，正好压在小地图上。
    // 只设 pointer-events:none 仍会留在 PrintWindow 帧里遮挡 MapLocator，
    // 必须直接隐藏；沿祖先链向上找会命中承载 #game-video 的全屏输入层。
    function disableCloudBall() {
        if (Date.now() - startedAt >= MAX_RUNTIME_MS) {
            if (ballTimer) {
                clearInterval(ballTimer);
            }
            return;
        }
        const nodes = document.querySelectorAll("body *");
        for (let i = 0; i < nodes.length; i += 1) {
            const node = nodes[i];
            const style = window.getComputedStyle(node);
            if (style.position !== "fixed" && style.position !== "absolute") {
                continue;
            }
            if (style.display === "none") {
                continue;
            }
            const rect = node.getBoundingClientRect();
            if (rect.width <= 0 || rect.height <= 0) {
                continue;
            }
            if (rect.width > 180 || rect.height > 180) {
                continue;
            }
            if (rect.right > 240 || rect.bottom > 240) {
                continue;
            }
            node.style.display = "none";
        }
    }

    function stop() {
        window.__maaCloudStart.running = false;
        clearInterval(timer);
        disableCloudBall();
        if (!ballTimer) {
            ballTimer = setInterval(disableCloudBall, 2000);
            window.__maaCloudStart.ballTimer = ballTimer;
        }
    }

    function isLoggedIn() {
        const text = document.body ? document.body.innerText : "";
        return LOGIN_MARKERS.some((marker) => text.indexOf(marker) !== -1);
    }

    // 落地页自带小尺寸预览 video；只有 canonical 视频且登录标记消失才算游戏已启动
    function isGameRunning() {
        const videos = document.querySelectorAll("video");
        let canonical = false;
        for (let i = 0; i < videos.length; i++) {
            if (videos[i].videoWidth >= 640 && videos[i].readyState >= 2 && !videos[i].paused) {
                canonical = true;
                break;
            }
        }
        return canonical && !isLoggedIn();
    }

    function isVisible(el) {
        if (!el || !el.getClientRects().length) {
            return false;
        }
        const style = window.getComputedStyle(el);
        return style.display !== "none" && style.visibility !== "hidden" && style.opacity !== "0";
    }

    // 精确匹配文本的可见叶子元素；隐藏的登录弹窗/按钮在 DOM 中仍存在，不能点击
    function findLeafByText(targetText) {
        const elements = document.querySelectorAll("*");
        for (let i = 0; i < elements.length; i++) {
            const el = elements[i];
            if (el.children.length !== 0 || !isVisible(el)) {
                continue;
            }
            if ((el.textContent || "").trim() === targetText) {
                return el;
            }
        }
        return null;
    }

    function clickLeaf(el) {
        const clickable = el.closest("button") || el;
        clickable.click();
        lastClickAt = Date.now();
    }

    function bodyHas(tokens) {
        const text = document.body ? document.body.innerText : "";
        return tokens.some((token) => text.indexOf(token) !== -1);
    }

    // 只解除开始按钮中心上的空文本透明遮罩；带文字的 SDK 登录/数据弹窗不动
    function neutralizeStartOverlay(button) {
        const rect = button.getBoundingClientRect();
        const x = Math.round(rect.x + rect.width / 2);
        const y = Math.round(rect.y + rect.height / 2);
        const at = document.elementFromPoint(x, y);
        if (!at || at === button || button.contains(at) || at.contains(button)) {
            return false;
        }
        if ((at.innerText || "").trim()) {
            return false;
        }
        at.style.pointerEvents = "none";
        return true;
    }

    function tick() {
        if (Date.now() - startedAt >= MAX_RUNTIME_MS) {
            stop();
            return;
        }
        if (isGameRunning()) {
            stop();
            return;
        }
        // 登录框存在且未见 UID 时 isLoggedIn 为 false，这里统一拦截，不做任何点击
        if (!isLoggedIn()) {
            return;
        }
        if (Date.now() - lastClickAt < CLICK_COOLDOWN_MS) {
            return;
        }
        const ack = findLeafByText(ACK_TEXT);
        if (ack) {
            clickLeaf(ack);
            return;
        }
        const login = findLeafByText(LOGIN_TEXT);
        if (login) {
            clickLeaf(login);
            return;
        }
        // 云端启动/排队/连接异常期间只等待，避免重复点击把会话打回落地页
        if (bodyHas(BUSY_TEXTS)) {
            return;
        }
        const start = findLeafByText(START_TEXT);
        if (start && startClicks < MAX_START_CLICKS && Date.now() - lastStartAt >= START_RETRY_MS) {
            neutralizeStartOverlay(start.closest("button") || start);
            clickLeaf(start);
            startClicks += 1;
            lastStartAt = Date.now();
        }
    }

    timer = setInterval(tick, POLL_INTERVAL_MS);
    window.__maaCloudStart.timer = timer;
})();
