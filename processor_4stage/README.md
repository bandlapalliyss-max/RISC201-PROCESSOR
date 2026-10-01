# 4-Stage RISC201 Pipelined Processor

## 1. Architectural Overview & Stage Decomposition

The standard baseline RISC pipeline (Smruti R. Sarangi, *Basic Computer Architecture*, Chapter 10) consists of 5 stages:
1. **IF** - Instruction Fetch
2. **OF** - Operand Fetch / Instruction Decode
3. **EX** - Execute (ALU & Branch Resolution)
4. **MA** - Memory Access
5. **RW** - Register Writeback

In this **4-Stage Design**, we merge **Memory Access (MA)** and **Register Writeback (RW)** into a unified **MA_RW** stage:
$$\text{IF} \longrightarrow \text{OF} \longrightarrow \text{EX} \longrightarrow \text{MA\_RW}$$

---

## 2. Rationale for Merging MA and RW (The Optimal Choice)

### Why not merge IF + OF?
Merging Instruction Fetch and Operand Fetch creates an extreme critical path bottleneck:
$$\text{Path: SRAM IMEM Access} \longrightarrow \text{Instruction Decoder} \longrightarrow \text{Dual-Ported Register File Read}$$
This would double the clock cycle time ($t_{clk}$), drastically lowering the operating frequency ($f = 1/t_{clk}$).

### Why not merge EX + MA?
Merging Execute and Memory Access would require computing the effective memory address via the ALU adder, and then immediately accessing the data cache/memory in the same clock period:
$$\text{Path: Address ALU Adder} \longrightarrow \text{SRAM Data Memory Access}$$
This would become the slowest stage in the entire processor, making the pipeline heavily unbalanced and violating the balanced-stage design principle ($t_{stage} \approx t_{max}/k + l$).

### Why MA + RW is the Best Combination:
1. **ALU Instructions Bypass Memory**: In SimpleRisc/RISC201, arithmetic and logical instructions do not access memory at all. In a 5-stage pipeline, they idle during the MA stage and only write back in RW.
2. **Single-Phase Memory and Latch-at-Edge**: For `ld` (Load), memory is read during the first part of the clock period, and the retrieved word is latched directly into the register file write port at the clock edge.
3. **Zero Register Writeback Overhead**: Register writeback is essentially just setting the register address and enabling the write strobe before the clock edge. It consumes almost negligible combinational delay ($< 5\%$ of cycle time).
4. **Reduced Branch Penalty**: With branch resolution in EX (Stage 3), a taken branch flushes only 2 stages (IF and OF), yielding a misprediction penalty of **2 cycles** instead of 3 cycles in deeper pipelines.
5. **Reduced Load-Use Hazard Penalty**: Forwarding from MA_RW directly to EX allows consecutive ALU instructions to execute without stalls, while load-use hazards require only **1 stall cycle**.

---

## 3. Data Forwarding & Hazard Resolution

The 4-stage processor implements full data forwarding and interlocks:
- **MA_RW $\rightarrow$ EX**: Forwards ALU result or completed memory load data directly into ALU input multiplexers $A$ and $B$.
- **MA_RW $\rightarrow$ OF**: Resolves simultaneous read/write to the same register in the register file.
- **Load-Use Hazard**: Detected when an instruction in EX is `ld` and the instruction in OF requires the loaded register (`rd == rs1` or `rd == rs2`). The hazard detection unit stalls IF and OF for 1 cycle and inserts a bubble into EX.
- **Control Hazards**: Handled with **Predict-Not-Taken**. When a branch is evaluated as taken in EX, the IF and OF stages are flushed (converted to bubbles), redirecting the PC to the branch target.
