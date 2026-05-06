# ASCII Speedup Breakdown - The Actual Numbers

## Your Corrected Data (From CECI A10):

```
ABSOLUTE SPEEDUP (How fast does the program run?)
═════════════════════════════════════════════════════

  1 thread  │ ███████████████████████████ 1.0×
  4 threads │ ██████████████████████████████████████████ 3.99× ✓✓✓ PERFECT
  8 threads │ ███████████████████████████████████████████████████████████ 7.59×  ✓✓
 16 threads │ ████████████████████████████████████████████████████████████████████████ 14.51× ✓


EFFICIENCY PER CORE (How much value does EACH core add?)
═════════════════════════════════════════════════════════

  1 thread  │ [████████████] 100% efficiency ← Every core at full strength
  4 threads │ [████████████] 99.7% efficiency ← Still perfect
  8 threads │ [████████████] 94.9% efficiency ← Declining
 16 threads │ [█████████░░░] 90.7% efficiency ← Diminished but still valuable
```

---

## Translation for Your Presentation

**What you said wrong:**
> "14× all the way to 16 cores"

**What you should say:**
> "We achieve 4× speedup at 4 cores with near-perfect efficiency, 7.6× at 8 cores, 
> and finally 14.5× at 16 cores. The efficiency per core drops from 100% to 91%, 
> showing that memory bandwidth becomes the limiting factor beyond 4 cores. 
> Nevertheless, all 16 cores remain useful."

---

## Why Both Are True

**Statement A:** "We only get 4× speedup at 4 cores" ✓ **CORRECT**
- This is absolute speedup: 1 thread takes time T, 4 threads take time T/4

**Statement B:** "We still benefit from 16 cores" ✓ **CORRECT**  
- Core 16 adds 0.9× per core = roughly +6% speedup vs 15 cores

**Statement C:** "14× speedup all the way" ✗ **WRONG**
- This conflates the final result with all intermediate results

---

## The Physics Explanation

```
Memory Bandwidth Available: ~100 GB/s (shared Intel bus)

1 core:   100 GB/s ÷ 1   = 100 GB/s per core  → Fully utilized
4 cores:  100 GB/s ÷ 4   = 25 GB/s per core   → Each core at full strength (✓ 4× speedup)
8 cores:  100 GB/s ÷ 8   = 12.5 GB/s per core → Contention begins (✓✓ 7.6× speedup)
16 cores: 100 GB/s ÷ 16  = 6.25 GB/s per core → Heavily contended (✓ 14.5× speedup)
```

Each core needs to **load 2 values per multiply** (A[i,k] and B[k,j]):

- At 25 GB/s: Can do ~50 multiplies per second per GB loaded = plenty of work
- At 6.25 GB/s: Can do only ~3 multiplies per second per GB = starved for work

**Result:** Cores still contribute (14.5× is great!), but with lower per-core value.

---

## Correct Table for Your Docs

| Cores | Total Speedup | Per-Core Efficiency | Status |
|-------|---------------|---------------------|--------|
| 1     | 1.0×          | 100%                | Baseline |
| 4     | **3.99×**     | **100%**            | ✓ Perfect scaling |
| 8     | **7.59×**     | **95%**             | ✓ Still strong |
| 16    | **14.51×**    | **91%**             | ✓ Valuable but memory-bound |

**Key insight:** Your code achieves 14.5× speedup. That's excellent! 
The efficiency curve shows why: 4 cores are magical (no contention), 
8 cores hit the wall, 16 cores are still worth it despite diminishing returns.

---

## One-Liner Fix for Your Presentation

**Before:**
> "14× all the way"

**After:**
> "Near-perfect scaling to 4 cores (4×), strong scaling to 8 cores (7.6×), 
> and memory-bound scaling to 16 cores (14.5×)"
