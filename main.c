import { useState, useCallback, useRef } from "react";

const COLS = 80;
const ROWS = 30;
const FILL = "*";
const BORDER = "_";
const EMPTY = " ";

// ─── Drawing algorithms ───────────────────────────────────────────────────────

function emptyGrid() {
  return Array.from({ length: ROWS }, () => Array(COLS).fill(EMPTY));
}

function setCell(grid, r, c, ch) {
  if (r >= 0 && r < ROWS && c >= 0 && c < COLS) grid[r][c] = ch;
}

function drawLine(grid, r1, c1, r2, c2) {
  const dr = Math.abs(r2 - r1), dc = Math.abs(c2 - c1);
  const sr = r1 < r2 ? 1 : -1, sc = c1 < c2 ? 1 : -1;
  let err = dr - dc, r = r1, c = c1;
  while (true) {
    setCell(grid, r, c, FILL);
    if (r === r2 && c === c2) break;
    const e2 = 2 * err;
    if (e2 > -dc) { err -= dc; r += sr; }
    if (e2 < dr)  { err += dr; c += sc; }
  }
}

function drawRect(grid, r1, c1, r2, c2) {
  const [rMin, rMax] = [Math.min(r1,r2), Math.max(r1,r2)];
  const [cMin, cMax] = [Math.min(c1,c2), Math.max(c1,c2)];
  for (let c = cMin; c <= cMax; c++) {
    setCell(grid, rMin, c, BORDER);
    setCell(grid, rMax, c, BORDER);
  }
  for (let r = rMin; r <= rMax; r++) {
    setCell(grid, r, cMin, BORDER);
    setCell(grid, r, cMax, BORDER);
  }
}

function drawCircle(grid, cr, cc, radius) {
  let x = 0, y = radius, d = 3 - 2 * radius;
  const plot8 = (dx, dy) => {
    [[cr+dy,cc+dx],[cr-dy,cc+dx],[cr+dy,cc-dx],[cr-dy,cc-dx],
     [cr+dx,cc+dy],[cr-dx,cc+dy],[cr+dx,cc-dy],[cr-dx,cc-dy]]
      .forEach(([r,c]) => setCell(grid, r, c, FILL));
  };
  while (y >= x) {
    plot8(x, y);
    x++;
    if (d > 0) { y--; d += 4*(x-y)+10; } else { d += 4*x+6; }
  }
}

function drawTriangle(grid, r1, c1, r2, c2, r3, c3) {
  drawLine(grid, r1, c1, r2, c2);
  drawLine(grid, r2, c2, r3, c3);
  drawLine(grid, r3, c3, r1, c1);
}

// ─── Render all objects to a fresh grid ──────────────────────────────────────

function renderObjects(objects) {
  const grid = emptyGrid();
  objects.forEach(obj => {
    const g = emptyGrid();
    if (obj.type === "line")
      drawLine(g, obj.r1, obj.c1, obj.r2, obj.c2);
    else if (obj.type === "rect")
      drawRect(g, obj.r1, obj.c1, obj.r2, obj.c2);
    else if (obj.type === "circle")
      drawCircle(g, obj.cr, obj.cc, obj.radius);
    else if (obj.type === "triangle")
      drawTriangle(g, obj.r1, obj.c1, obj.r2, obj.c2, obj.r3, obj.c3);
    for (let r = 0; r < ROWS; r++)
      for (let c = 0; c < COLS; c++)
        if (g[r][c] !== EMPTY) grid[r][c] = g[r][c];
  });
  return grid;
}

// ─── Shape form components ───────────────────────────────────────────────────

const F = ({ label, value, onChange, min = 0, max }) => (
  <label style={{ display:"flex", flexDirection:"column", gap:2, fontSize:12, color:"#aaa" }}>
    {label}
    <input type="number" min={min} max={max ?? (label.toLowerCase().includes("col") ? COLS-1 : ROWS-1)}
      value={value}
      onChange={e => onChange(Number(e.target.value))}
      style={{ width:56, background:"#1a1a2e", border:"1px solid #333", color:"#e0e0ff",
               borderRadius:4, padding:"3px 6px", fontSize:13 }} />
  </label>
);

function LineForm({ init, onSave }) {
  const d = init || { r1:5, c1:5, r2:10, c2:20 };
  const [s, set] = useState(d);
  return (
    <div style={{ display:"flex", gap:10, flexWrap:"wrap", alignItems:"flex-end" }}>
      <F label="R1" value={s.r1} onChange={v=>set({...s,r1:v})} />
      <F label="C1" value={s.c1} onChange={v=>set({...s,c1:v})} />
      <F label="R2" value={s.r2} onChange={v=>set({...s,r2:v})} />
      <F label="C2" value={s.c2} onChange={v=>set({...s,c2:v})} />
      <button onClick={() => onSave({type:"line",...s})} style={btnStyle("#7c5cbf")}>Save</button>
    </div>
  );
}

function RectForm({ init, onSave }) {
  const d = init || { r1:3, c1:5, r2:12, c2:25 };
  const [s, set] = useState(d);
  return (
    <div style={{ display:"flex", gap:10, flexWrap:"wrap", alignItems:"flex-end" }}>
      <F label="Top R" value={s.r1} onChange={v=>set({...s,r1:v})} />
      <F label="Left C" value={s.c1} onChange={v=>set({...s,c1:v})} />
      <F label="Bot R" value={s.r2} onChange={v=>set({...s,r2:v})} />
      <F label="Right C" value={s.c2} onChange={v=>set({...s,c2:v})} />
      <button onClick={() => onSave({type:"rect",...s})} style={btnStyle("#7c5cbf")}>Save</button>
    </div>
  );
}

function CircleForm({ init, onSave }) {
  const d = init || { cr:12, cc:20, radius:8 };
  const [s, set] = useState(d);
  return (
    <div style={{ display:"flex", gap:10, flexWrap:"wrap", alignItems:"flex-end" }}>
      <F label="Center R" value={s.cr} onChange={v=>set({...s,cr:v})} />
      <F label="Center C" value={s.cc} onChange={v=>set({...s,cc:v})} />
      <F label="Radius" value={s.radius} onChange={v=>set({...s,radius:v})} min={1} max={14} />
      <button onClick={() => onSave({type:"circle",...s})} style={btnStyle("#7c5cbf")}>Save</button>
    </div>
  );
}

function TriangleForm({ init, onSave }) {
  const d = init || { r1:2, c1:20, r2:14, c2:5, r3:14, c3:35 };
  const [s, set] = useState(d);
  return (
    <div style={{ display:"flex", gap:10, flexWrap:"wrap", alignItems:"flex-end" }}>
      {["r1","c1","r2","c2","r3","c3"].map(k => (
        <F key={k} label={k.toUpperCase()} value={s[k]} onChange={v=>set({...s,[k]:v})} />
      ))}
      <button onClick={() => onSave({type:"triangle",...s})} style={btnStyle("#7c5cbf")}>Save</button>
    </div>
  );
}

const SHAPE_FORMS = { line: LineForm, rect: RectForm, circle: CircleForm, triangle: TriangleForm };

function btnStyle(bg="#333") {
  return { background:bg, color:"#fff", border:"none", borderRadius:5,
           padding:"6px 14px", cursor:"pointer", fontSize:13, fontWeight:600, whiteSpace:"nowrap" };
}

// ─── Main App ─────────────────────────────────────────────────────────────────

let idCounter = 1;

export default function App() {
  const [objects, setObjects] = useState([]);
  const [shapeType, setShapeType] = useState("circle");
  const [editId, setEditId] = useState(null);
  const [addKey, setAddKey] = useState(0); // forces re-mount of form

  const grid = renderObjects(objects);

  const handleAdd = (obj) => {
    setObjects(prev => [...prev, { ...obj, id: idCounter++ }]);
    setAddKey(k => k+1);
  };

  const handleDelete = (id) => {
    setObjects(prev => prev.filter(o => o.id !== id));
    if (editId === id) setEditId(null);
  };

  const handleModify = (updated) => {
    setObjects(prev => prev.map(o => o.id === editId ? { ...updated, id: o.id } : o));
    setEditId(null);
  };

  const Form = SHAPE_FORMS[shapeType];
  const editObj = objects.find(o => o.id === editId);
  const EditForm = editObj ? SHAPE_FORMS[editObj.type] : null;

  const shapeLabel = { line:"Line", rect:"Rectangle", circle:"Circle", triangle:"Triangle" };

  return (
    <div style={{ fontFamily:"'Courier New', monospace", background:"#0d0d1a",
                  minHeight:"100vh", color:"#e0e0ff", padding:"18px 16px" }}>

      <div style={{ marginBottom:14 }}>
        <span style={{ fontSize:20, fontWeight:700, letterSpacing:2, color:"#a78bfa" }}>
          ✦ ASCII GRAPHICS EDITOR
        </span>
        <span style={{ marginLeft:14, fontSize:12, color:"#555" }}>
          canvas: {COLS}×{ROWS} · chars: <span style={{color:"#a78bfa"}}>*</span> fill · <span style={{color:"#7dd3fc"}}>_</span> border
        </span>
      </div>

      {/* Canvas */}
      <div style={{ overflowX:"auto", marginBottom:16 }}>
        <pre style={{
          background:"#070714", border:"1px solid #2a2a4a", borderRadius:8,
          padding:"10px 12px", lineHeight:1.35, fontSize:13,
          display:"inline-block", minWidth:"fit-content",
          boxShadow:"0 0 30px #a78bfa22"
        }}>
          {grid.map((row, r) => (
            <span key={r} style={{ display:"block" }}>
              {row.map((ch, c) => (
                <span key={c} style={{
                  color: ch === FILL ? "#a78bfa" : ch === BORDER ? "#7dd3fc" : "#1a1a2e"
                }}>{ch}</span>
              ))}
            </span>
          ))}
        </pre>
      </div>

      {/* Add Shape Panel */}
      <div style={{ background:"#12122a", border:"1px solid #2a2a4a", borderRadius:8,
                    padding:"14px 16px", marginBottom:12 }}>
        <div style={{ fontSize:12, color:"#a78bfa", fontWeight:700,
                      letterSpacing:1, marginBottom:10 }}>ADD SHAPE</div>
        <div style={{ display:"flex", gap:8, marginBottom:12, flexWrap:"wrap" }}>
          {Object.keys(SHAPE_FORMS).map(t => (
            <button key={t} onClick={() => { setShapeType(t); setAddKey(k=>k+1); }}
              style={{
                ...btnStyle(shapeType===t ? "#7c5cbf" : "#1e1e3a"),
                border: shapeType===t ? "1px solid #a78bfa" : "1px solid #2a2a4a"
              }}>
              {shapeLabel[t]}
            </button>
          ))}
        </div>
        <Form key={`add-${shapeType}-${addKey}`} onSave={handleAdd} />
      </div>

      {/* Edit Panel */}
      {EditForm && (
        <div style={{ background:"#0e1e2e", border:"1px solid #7dd3fc55", borderRadius:8,
                      padding:"14px 16px", marginBottom:12 }}>
          <div style={{ display:"flex", justifyContent:"space-between", alignItems:"center", marginBottom:10 }}>
            <span style={{ fontSize:12, color:"#7dd3fc", fontWeight:700, letterSpacing:1 }}>
              EDITING — {editObj.type.toUpperCase()} #{editId}
            </span>
            <button onClick={() => setEditId(null)} style={btnStyle("#333")}>✕ Cancel</button>
          </div>
          <EditForm key={`edit-${editId}`} init={editObj} onSave={handleModify} />
        </div>
      )}

      {/* Objects List */}
      <div style={{ background:"#12122a", border:"1px solid #2a2a4a", borderRadius:8,
                    padding:"14px 16px" }}>
        <div style={{ fontSize:12, color:"#a78bfa", fontWeight:700,
                      letterSpacing:1, marginBottom:10 }}>
          OBJECTS ({objects.length})
        </div>
        {objects.length === 0 && (
          <div style={{ color:"#444", fontSize:13, padding:"6px 0" }}>
            No objects yet. Add a shape above.
          </div>
        )}
        <div style={{ display:"flex", flexDirection:"column", gap:6 }}>
          {objects.map(obj => {
            const desc = obj.type === "line"
              ? `(${obj.r1},${obj.c1}) → (${obj.r2},${obj.c2})`
              : obj.type === "rect"
              ? `(${obj.r1},${obj.c1}) to (${obj.r2},${obj.c2})`
              : obj.type === "circle"
              ? `center (${obj.cr},${obj.cc}) r=${obj.radius}`
              : `(${obj.r1},${obj.c1}) (${obj.r2},${obj.c2}) (${obj.r3},${obj.c3})`;
            const isEditing = editId === obj.id;
            return (
              <div key={obj.id}
                style={{ display:"flex", alignItems:"center", gap:8, padding:"7px 10px",
                         background: isEditing ? "#1a2e3a" : "#0d0d22",
                         border:`1px solid ${isEditing?"#7dd3fc":"#222"}`,
                         borderRadius:6, flexWrap:"wrap" }}>
                <span style={{ color:"#555", fontSize:11, minWidth:24 }}>#{obj.id}</span>
                <span style={{ color:"#a78bfa", fontSize:12, fontWeight:600, minWidth:60 }}>
                  {obj.type.toUpperCase()}
                </span>
                <span style={{ color:"#888", fontSize:12, flex:1 }}>{desc}</span>
                <button onClick={() => setEditId(isEditing ? null : obj.id)}
                  style={btnStyle(isEditing?"#1a4a6a":"#1e1e3a")}>
                  {isEditing ? "↑ Editing" : "✎ Edit"}
                </button>
                <button onClick={() => handleDelete(obj.id)}
                  style={btnStyle("#3a1a1a")}>✕ Del</button>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}
