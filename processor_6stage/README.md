# 6-Stage RISC201 Pipelined Processor

## 1. Architectural Overview & Stage Decomposition

The standard baseline RISC pipeline (Smruti R. Sarangi, *Basic Computer Architecture*, Chapter 10) consists of 5 stages:
1. **IF** - Instruction Fetch
2. **OF** - Operand Fetch / Instruction Decode
3. **EX** - Execute
4. **MA** - Memory Access
5. **RW** - Register Writeback

In this **6-Stage Design**, the **Operand Fetch (OF)** stage is split into two distinct, balanced sub-stages:
1. **ID** - Instruction Decode & Immediate Generation
2. **RF** - Register Fetch & Operand Routing

The resulting 6-stage pipeline is:
$$\text{IF} \longrightarrow \text{ID} \longrightarrow \text{RF} \longrightarrow \text{EX} \longrightarrow \text{MA} \longrightarrow \text{RW}$$

---

## 2. Rationale for Splitting OF into ID and RF (The Optimal Choice)

### Why not split EX into EX1 + EX2?
While splitting EX is sometimes done for complex floating-point or iterative multipliers, standard SimpleRisc/RISC201 ALU operations (addition, subtraction, logical AND/OR, shifts) are single-cycle operations. Dividing the integer ALU into two stages would introduce structural pipeline bubbles for simple arithmetic instructions and double the forwarding network complexity.

### Why not split MA into MA1 + MA2?
A 2-cycle memory pipeline is common when dealing with large L1 caches or TLB tag-compare followed by data array access. However, for a basic RISC201 SRAM memory model, splitting memory creates load-use penalties of 2 or 3 cycles, degrading CPI on memory-intensive codes without benefiting ALU-heavy integer loops.

### Why Splitting OF into ID + RF is the Best Choice:
1. **Critical Path Decomposition**:
   - In a unified OF stage, the hardware must perform:
     $$\text{Opcode decode} \longrightarrow \text{Modifier decoding} \longrightarrow \text{Sign extension / Shift} \longrightarrow \text{Mux register addresses} \longrightarrow \text{SRAM Register File Read}$$
   - In modern high-frequency processor implementations (e.g. ARM Cortex-A8, MIPS R4000), accessing the multi-ported SRAM register file is a major delay component.
   - Decoupling instruction decoding and immediate generation (**ID**) from multi-port register array access and operand forwarding (**RF**) creates two balanced stages, substantially lowering the stage delay $t_{stage}$ and permitting higher clock frequency ($f$).
2. **Modular Front-End**:
   - Instruction decoding and immediate calculation are purely combinational operations based on the instruction word and current PC.
   - Register access depends on the decoded specifiers and requires interfacing with the register file array and the writeback forwarding bus. Separating these concerns improves timing margin and eliminates race conditions.

---

## 3. Hazards and Forwarding in the 6-Stage Pipeline

- **Data Forwarding Paths**:
  - **EX $\rightarrow$ EX** (via `EX_MA` latch, distance 1): Forwards ALU result to the immediately following instruction entering EX.
  - **MA $\rightarrow$ EX** (via `MA_RW` latch, distance 2): Forwards memory load result or earlier ALU result to an instruction in EX.
  - **RW $\rightarrow$ RF** (distance 3): Forwards writeback data to the register fetch stage when an instruction is writing to a register that a later instruction is reading in RF.
  - **MA $\rightarrow$ MA**: Forwards store data directly into the memory stage.
- **Load-Use Hazard**:
  - The load instruction produces data at the end of MA.
  - A consumer instruction needs the value at the beginning of EX.
  - A 1-cycle stall is inserted (stalling IF, ID, and RF, while inserting a bubble into EX).
- **Branch Penalty (Control Hazard)**:
  - Branch condition and target are resolved in EX (Stage 4).
  - With Predict-Not-Taken, if a branch is taken, 3 instructions are in flight on the wrong path (in IF, ID, and RF).
  - Branch misprediction penalty is **3 cycles** (flushing IF_ID, ID_RF, and RF_EX).

---

## 4. 4-Stage vs 6-Stage Comparative Tradeoff

| Metric | 4-Stage (Merged MA_RW) | 6-Stage (Split ID/RF) |
| :--- | :--- | :--- |
| **Pipeline Depth** | 4 stages | 6 stages |
| **Branch Penalty** | 2 cycles | 3 cycles |
| **Stage Delay ($t_{stage}$)** | Longer (Memory + RegFile setup) | Shorter (Balanced sub-stages) |
| **Max Clock Freq ($f$)** | Moderate | Higher |
| **CPI on Branch-heavy Code** | Lower CPI (fewer flush cycles) | Higher CPI (3 bubbles per taken branch) |
| **Throughput ($f \times \text{IPC}$)** | Superior on short/branch-heavy code | Superior on compute-heavy straight-line code |
