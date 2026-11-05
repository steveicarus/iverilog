# 8×8 Binary Multiplier on FPGA

A hardware implementation of an **8-bit × 8-bit binary multiplier** using an **AMD Xilinx Spartan-7 FPGA** and a 4-digit 7-segment display.

The design accepts two independent 8-bit binary inputs through **16 physical switches**, performs multiplication in FPGA logic, converts the result for decimal display, and displays the result on a 4-digit seven-segment display.

---

## 🛠️ Tools & Hardware

### FPGA Board

**EDGE Development Board – AMD Xilinx Spartan-7**

- FPGA: **AMD Xilinx XC7S15FTGB196-1**
- FPGA Family: Spartan-7
- Package: FTGB196
- Tool: **AMD Xilinx Vivado 2025 Edition**

---

## 🔢 Input Configuration

The design uses **16 physical switches** divided into two independent 8-bit input sets.

### Set A — 8-bit Input

| Bit      | FPGA Pin |   Weight |
| -------- | -------- | -------: |
| A0 (LSB) | J1       |   2⁰ = 1 |
| A1       | K3       |   2¹ = 2 |
| A2       | J3       |   2² = 4 |
| A3       | L1       |   2³ = 8 |
| A4       | M2       |  2⁴ = 16 |
| A5       | N1       |  2⁵ = 32 |
| A6       | P3       |  2⁶ = 64 |
| A7 (MSB) | L2       | 2⁷ = 128 |

### Set B — 8-bit Input

| Bit      | FPGA Pin |   Weight |
| -------- | -------- | -------: |
| B0 (LSB) | N4       |   2⁰ = 1 |
| B1       | M10      |   2¹ = 2 |
| B2       | P10      |   2² = 4 |
| B3       | N10      |   2³ = 8 |
| B4       | P12      |  2⁴ = 16 |
| B5       | N14      |  2⁵ = 32 |
| B6       | M11      |  2⁶ = 64 |
| B7 (MSB) | K11      | 2⁷ = 128 |

---

## ⚙️ Input Representation

Each switch represents one binary bit.

- **OFF = 0**
- **ON = 1**

Each 8-switch set can represent values from:

```text
00000000 = 0
to
11111111 = 255

```
<img width="1600" height="1200" alt="WhatsApp Image 2026-08-30 at 12 10 15 PM" src="https://github.com/user-attachments/assets/97f3b9e2-8ade-4906-818e-b500f6b4f0c3" />
