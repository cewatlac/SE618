# SE618 Lecture Slides

Lectures 1–14 are complete (Lecture 7: 80-minute block after the midterm; Lectures 13–14: defense sessions, with modular synthesis and final-review material in 14). Each `Lecture_NN/index.html` is a self-contained 3-hour session with instructor notes; `common/` holds the shared engine. No build step, no internet needed.

## Presenting

Open `Lecture_NN/index.html` in Chrome or Edge (double-click works).

| Key | Action |
|---|---|
| → / Space / PgDn, ← / PgUp | Next / previous slide |
| Home / End | First / last slide |
| **N** | Toggle the **Instructor Notes** panel (laptop screen only — do not mirror) |
| **S** | Open the **presenter window** (current slide, next slide, notes, timer). Put it on your laptop and the main window on the projector (extended display). Either window navigates both. |
| G | Contents (jump to any slide, grouped by section) |
| F | Fullscreen · B blackout |

The URL hash (`#12`) is the slide number, so a reload keeps your place.

## Sharing with students (important)

The notes contain expected answers and lab solutions. **Do not upload `index.html` to Classroom.** Instead click **Print** (slides only) → *Save as PDF* → upload the PDF. **Print + Notes** produces an A4 instructor handout (slide thumbnail + full notes per page). Use Chrome/Edge; Firefox ignores the A4 page switch.

Each lecture's `Student_Resources/` holds only files that are safe to give students. Solutions and instructor-only files live in the top-level `Instructor_Only/Lecture_NN/` — never upload that folder; release individual solutions after the session if you choose.

## Authoring additional lectures

Copy `_template/index.html` to `Lecture_NN/index.html`, update the `<title>` and `data-lecture`, and write slides:

```html
<section class="slide divider"><span class="kicker">Part 2 · 00:20–01:00</span><h1>Section title</h1></section>

<section class="slide">
  <h2>Slide title</h2>
  <div class="cols">…</div>                          <!-- .cols, .cols.c3, .cols.wide-l / .wide-r -->
  <div class="box key|warn|q|good|bad|plain">…</div> <!-- optional <span class="label"> -->
  <pre><code>// C++ is highlighted automatically</code></pre>
  <aside class="notes"><p><span class="t">00:25</span></p><h4>Say</h4><p>…</p></aside>
</section>
```

Canvas is 1280×720; text classes `.small`, `.xsmall`, `.lead`, `.muted`; tables accept `.tight`, `tr.hl`, `td.num`. Open the page and check the console: `SE618 slides overflowing: …` lists any slide whose content does not fit (also marked with a red "overflow" tag).

Notes convention: timing chip `<span class="t">hh:mm</span>`; headings *Say*, *Ask*, *Expected answer*, *Misconception*, *Transition*; solutions only in notes.

Outlines from the first draft are in `../_legacy_draft_v1/lectures/` (Lecture 12 of that draft was the demo/review session).
