/* SE618 slide engine: no dependencies, works from file://.
   Keys: →/Space/PgDn next · ←/PgUp prev · Home/End · N notes · S presenter window · G contents
         F fullscreen · B blackout · Esc close panels. URL hash #12 = slide 12. */
(function () {
  "use strict";
  const params = new URLSearchParams(location.search);
  const isPresenter = false; // Public student release
  const body = document.body;
  // Public version: instructor handouts are not available.
  const slides = Array.from(document.querySelectorAll("section.slide"));
  const course = body.dataset.course || "SE618";
  const lecture = body.dataset.lecture || "";
  let cur = 0, peer = null;

  /* ---------- build structure ---------- */
  let section = "";
  slides.forEach((s, i) => {
    const notes = s.querySelector(":scope > aside.notes");
    const frame = document.createElement("div");
    frame.className = "frame";
    Array.from(s.childNodes).forEach(n => { if (n !== notes) frame.appendChild(n); });
    s.insertBefore(frame, notes || null);
    if (s.classList.contains("divider")) section = (s.querySelector("h1") || {}).textContent || section;
    s.dataset.section = s.dataset.section || section;
    const h = s.querySelector("h2, h1");
    s.dataset.heading = s.dataset.title || (h ? h.textContent.trim() : "Slide " + (i + 1));
    const foot = document.createElement("div");
    foot.className = "foot";
    foot.innerHTML = "<span>" + course + " · " + lecture + "</span><span>" + (i + 1) + " / " + slides.length + "</span>";
    frame.appendChild(foot);
  });

  const viewport = document.createElement("div");
  viewport.id = "viewport";
  slides[0].parentNode.insertBefore(viewport, slides[0]);
  slides.forEach(s => viewport.appendChild(s));

  document.querySelectorAll(".frame pre code, .frame pre").forEach(el => {
    if (el.tagName === "PRE" && el.querySelector("code")) return;
    if (el.closest("pre.nohl")) return;
    if (!el.dataset.hl) { el.dataset.hl = 1; highlight(el); }
  });

  body.insertAdjacentHTML("beforeend",
    '<div id="progress"></div>' +
    '<div id="notes-panel"><div class="notes-banner">Instructor notes — not for projection</div><div class="notes-body"></div></div>' +
    '<div id="jump"></div><div id="blackout"></div>' +
    '<div id="toolbar"><button data-a="prev" title="Previous (←)">◀</button><span class="count"></span>' +
    '<button data-a="next" title="Next (→)">▶</button><span class="sec"></span>' +
    '<button data-a="jump" title="Contents (G)">Contents</button>' +
    '<button data-a="print" title="Print slides only / Save as PDF">Print</button>' +
    '<span class="hint">←/→ slides · G contents · F fullscreen</span></div>' +
    '<div id="pv-top"><button data-a="prev">◀ Prev</button><button data-a="next">Next ▶</button>' +
    '<span class="count"></span><span class="timer">00:00:00</span><button data-a="reset">Reset timer</button>' +
    '<span class="clock"></span><span class="sec" style="flex:1;color:#9aa6b3"></span></div>' +
    '<div id="pv-next"><span class="lbl">NEXT</span></div>');

  /* Public course navigation: same relative paths on local files and GitHub Pages. */
  const match = location.pathname.match(/Lecture_(\d{2})\/index\.html$/);
  const lectureNumber = match ? Number(match[1]) : Number((body.dataset.lecture || "").match(/\d+/)?.[0] || 1);
  const toolbar = document.getElementById("toolbar");
  const nav = document.createElement("nav");
  nav.className = "course-nav";
  const pad = n => String(n).padStart(2,"0");
  nav.innerHTML = '<a class="nav-library" href="../index.html" title="Course library">← Library</a>' +
    (lectureNumber > 1 ? '<a class="nav-adjacent" href="../Lecture_' + pad(lectureNumber-1) + '/index.html" title="Previous lecture">‹ L' + pad(lectureNumber-1) + '</a>' : '') +
    (lectureNumber < 14 ? '<a class="nav-adjacent" href="../Lecture_' + pad(lectureNumber+1) + '/index.html" title="Next lecture">L' + pad(lectureNumber+1) + ' ›</a>' : '');
  toolbar.insertBefore(nav, toolbar.firstChild);
  const resources = document.createElement("a");
  resources.className = "resource-nav";
  resources.href = "../index.html#lecture-" + pad(lectureNumber);
  resources.textContent = "Materials ↗";
  toolbar.insertBefore(resources, toolbar.lastElementChild);

  const notesBody = document.querySelector("#notes-panel .notes-body");
  const progress = document.getElementById("progress");

  /* ---------- contents menu ---------- */
  const jump = document.getElementById("jump");
  let lastSec = null, html = "";
  slides.forEach((s, i) => {
    if (s.dataset.section !== lastSec) { lastSec = s.dataset.section; html += "<h3>" + esc(lastSec || "Opening") + "</h3>"; }
    html += '<a href="#' + (i + 1) + '" data-i="' + i + '"><span>' + (i + 1) + "</span>" + esc(s.dataset.heading) + "</a>";
  });
  jump.innerHTML = html;
  jump.addEventListener("click", e => {
    const a = e.target.closest("a"); if (!a) return;
    e.preventDefault(); body.classList.remove("jump-open"); go(+a.dataset.i, true);
  });

  /* ---------- navigation ---------- */
  function go(i, local) {
    i = Math.max(0, Math.min(slides.length - 1, i));
    slides[cur].classList.remove("current");
    cur = i;
    const s = slides[cur];
    s.classList.add("current");
    fit();
    document.querySelectorAll(".count").forEach(c => c.textContent = (cur + 1) + " / " + slides.length);
    document.querySelectorAll("#toolbar .sec, #pv-top .sec").forEach(c => c.textContent = s.dataset.section || "");
    progress.style.width = ((cur + 1) / slides.length * 100) + "%";
    const n = s.querySelector("aside.notes");
    notesBody.innerHTML = "<h3 style='margin:4px 0 6px;font-size:18px'>" + (cur + 1) + ". " + esc(s.dataset.heading) + "</h3>" +
      (n && n.innerHTML.trim() ? n.innerHTML : "<p class='notes-empty'>No notes for this slide.</p>");
    notesBody.parentNode.scrollTop = 0;
    jump.querySelectorAll("a").forEach(a => a.classList.toggle("cur", +a.dataset.i === cur));
    if (history.replaceState) history.replaceState(null, "", location.pathname + location.search + "#" + (cur + 1));
    if (isPresenter) renderNext();
    if (local && peer && !peer.closed) peer.postMessage({ se618: "goto", i: cur }, "*");
  }

  function fit() {
    const r = viewport.getBoundingClientRect();
    const k = Math.min(r.width / 1280, r.height / 720) * 0.98;
    slides[cur].style.transform = "scale(" + k + ")";
  }

  function renderNext() {
    const box = document.getElementById("pv-next");
    box.querySelectorAll(".slide").forEach(n => n.remove());
    if (cur + 1 >= slides.length) return;
    const c = slides[cur + 1].cloneNode(true);
    c.classList.remove("current");
    box.appendChild(c);
    const r = box.getBoundingClientRect();
    c.style.transform = "scale(" + Math.min(r.width / 1280, r.height / 720) * 0.92 + ")";
  }

  function act(a) {
    if (a === "next") go(cur + 1, true);
    else if (a === "prev") go(cur - 1, true);
    else if (a === "notes") { body.classList.toggle("notes-open"); btn("notes", body.classList.contains("notes-open")); fit(); }
    else if (a === "jump") body.classList.toggle("jump-open");
    else if (a === "presenter") openPresenter();
    else if (a === "print") { body.classList.remove("print-notes"); window.print(); }
    else if (a === "printnotes") { body.classList.add("print-notes"); window.print(); }
    else if (a === "reset") t0 = Date.now();
  }
  function btn(a, on) { const b = document.querySelector('#toolbar [data-a="' + a + '"]'); if (b) b.classList.toggle("on", on); }
  window.addEventListener("afterprint", () => { if (!params.has("printnotes")) body.classList.remove("print-notes"); });

  document.addEventListener("click", e => { const b = e.target.closest("[data-a]"); if (b) act(b.dataset.a); });
  document.addEventListener("keydown", e => {
    if (e.ctrlKey || e.metaKey || e.altKey) return;
    const k = e.key;
    if (["ArrowRight", "PageDown", " ", "ArrowDown"].includes(k)) { e.preventDefault(); act("next"); }
    else if (["ArrowLeft", "PageUp", "ArrowUp", "Backspace"].includes(k)) { e.preventDefault(); act("prev"); }
    else if (k === "Home") go(0, true);
    else if (k === "End") go(slides.length - 1, true);
    else if (k === "g" || k === "G") act("jump");
    else if (k === "b" || k === "B" || k === ".") body.classList.toggle("black");
    else if (k === "f" || k === "F") { if (!document.fullscreenElement) document.documentElement.requestFullscreen().catch(() => {}); else document.exitFullscreen(); }
    else if (k === "Escape") body.classList.remove("jump-open", "black");
  });
  let tx = null;
  viewport.addEventListener("touchstart", e => { tx = e.touches[0].clientX; }, { passive: true });
  viewport.addEventListener("touchend", e => {
    if (tx === null) return; const dx = e.changedTouches[0].clientX - tx; tx = null;
    if (Math.abs(dx) > 50) act(dx < 0 ? "next" : "prev");
  });
  window.addEventListener("resize", () => { fit(); if (isPresenter) renderNext(); });

  /* ---------- presenter window (postMessage works on file://) ---------- */
  function openPresenter() {
    if (isPresenter) return;
    const url = location.href.split("#")[0].split("?")[0] + "?presenter#" + (cur + 1);
    peer = window.open(url, "se618-presenter", "width=1400,height=860");
  }
  window.addEventListener("message", e => {
    const d = e.data; if (!d || !d.se618) return;
    if (d.se618 === "goto") { if (e.source) peer = e.source; go(d.i, false); }
    if (d.se618 === "hello" && e.source) { peer = e.source; peer.postMessage({ se618: "goto", i: cur }, "*"); }
  });
  let t0 = Date.now();
  if (isPresenter) {
    body.classList.add("presenter");
    document.title = "Presenter — " + document.title;
    if (window.opener) { peer = window.opener; peer.postMessage({ se618: "hello" }, "*"); }
    setInterval(() => {
      const s = Math.floor((Date.now() - t0) / 1000);
      document.querySelector("#pv-top .timer").textContent =
        [s / 3600, (s % 3600) / 60, s % 60].map(v => String(Math.floor(v)).padStart(2, "0")).join(":");
      document.querySelector("#pv-top .clock").textContent = new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
    }, 1000);
  }

  /* ---------- overflow check (authoring aid; prints a console warning) ---------- */
  function checkOverflow() {
    const bad = [];
    slides.forEach((s, i) => {
      const was = s.style.display; s.style.display = "block";
      const f = s.querySelector(".frame");
      const over = f.scrollHeight > f.clientHeight + 2 || f.scrollWidth > f.clientWidth + 2;
      let deep = false;
      f.querySelectorAll("pre").forEach(p => { if (p.scrollWidth > p.clientWidth + 2) deep = true; });
      s.style.display = was;
      if (over || deep) { s.dataset.overflow = over ? "frame" : "code"; bad.push(i + 1); }
    });
    if (bad.length) console.warn("SE618 slides overflowing:", bad.join(", "));
    window.SE618_OVERFLOW = bad;
  }

  /* ---------- tiny C++ highlighter ---------- */
  function highlight(el) {
    const src = el.textContent;
    const re = /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])')|(^[ \t]*#[^\n]*)|\b(\d[\d'.]*[uUlLfF]*)\b|\b(alignas|auto|bool|break|case|catch|char|class|const|constexpr|continue|default|delete|do|double|else|enum|explicit|false|float|for|friend|if|inline|int|long|mutable|namespace|new|noexcept|nullptr|operator|private|protected|public|return|short|signed|sizeof|static|static_assert|struct|switch|template|this|throw|true|try|typename|unsigned|using|virtual|void|while|override|final|concept|requires)\b|\b(std::[A-Za-z_][\w:]*|size_t|uint\d+_t|int\d+_t|string_view|string)\b/gm;
    let out = "", last = 0, m;
    while ((m = re.exec(src))) {
      out += esc(src.slice(last, m.index));
      const cls = m[1] ? "c" : m[2] ? "s" : m[3] ? "p" : m[4] ? "n" : m[5] ? "k" : "t";
      out += '<span class="tok-' + cls + '">' + esc(m[0]) + "</span>";
      last = re.lastIndex;
    }
    el.innerHTML = out + esc(src.slice(last));
  }
  function esc(s) { return String(s).replace(/[&<>"]/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c])); }

  window.addEventListener("hashchange", () => {
    const h = parseInt(location.hash.slice(1), 10);
    if (!isNaN(h) && h - 1 !== cur) go(h - 1, true);
  });
  const start = parseInt((location.hash || "").slice(1), 10);
  slides[0].classList.add("current");
  go(isNaN(start) ? 0 : start - 1, false);
  if (document.readyState === "complete") checkOverflow(); else window.addEventListener("load", checkOverflow);
})();
