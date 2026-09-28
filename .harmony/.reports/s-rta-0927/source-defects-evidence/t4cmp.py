"""T4 compare: A1/A2 = two runs of the main (pre-change) app -> noise floor; B = the lane app.
usage: t4cmp.py <A1 dir> <A2 dir> <B dir> <out.json>. Prints the table of every capture that is not byte-identical."""
import sys, os, json, cv2, numpy as np
A1, A2, B, OUTJ = sys.argv[1:5]
def ps(a, b):
    if a is None or b is None: return None
    if a.shape != b.shape: return -1.0
    mse = ((a.astype(np.float64) - b.astype(np.float64)) ** 2).mean()
    return float("inf") if mse == 0 else float(10 * np.log10(255 * 255 / mse))
names = sorted(set(os.listdir(A1 + "/png")) | set(os.listdir(B + "/png")))
rows = []
for n in names:
    a1 = cv2.imread(f"{A1}/png/{n}"); a2 = cv2.imread(f"{A2}/png/{n}"); b = cv2.imread(f"{B}/png/{n}")
    noise = ps(a1, a2); d = ps(a1, b)
    rows.append({"name": n, "noise": noise, "lane": d})
json.dump(rows, open(OUTJ, "w"), indent=0, default=str)
det = [r for r in rows if r["noise"] == float("inf")]
nondet = [r for r in rows if r["noise"] != float("inf")]
print(f"captures: {len(rows)}; deterministic on main (run1 == run2 byte-identical): {len(det)}; non-deterministic: {len(nondet)}")
print("non-deterministic on main:", ", ".join(f"{r['name']}({r['noise']:.1f})" for r in nondet if r['noise'] is not None))
changed = [r for r in det if r["lane"] != float("inf")]
print(f"deterministic captures that CHANGED on the lane build: {len(changed)}")
for r in changed: print(f"  CHANGED {r['name']:60s} PSNR main->lane = {r['lane']}")
ndfail = [r for r in nondet if r["lane"] is None or r["lane"] < 30]
print(f"non-deterministic captures with PSNR(main, lane) < 30: {len(ndfail)}")
for r in ndfail: print(f"  LOW {r['name']:60s} noise={r['noise']} lane={r['lane']}")
missing = [r for r in rows if r["lane"] is None or r["noise"] is None]
print("missing on one side:", [r["name"] for r in missing])
