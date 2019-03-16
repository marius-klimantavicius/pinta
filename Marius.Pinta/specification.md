# Pinta Virtual Machine Specification

This document describes Pinta Virtual Machine (VM in later sections) execution environment.
The specification is divided into two sections - the first describes executable file format used by the VM, the second part describes supported instructions and semantics.

## Executable file format

## Execution and instructions

The VM is stack based vm with automatic memory management.

### Arithmetic operations

Arithmetic operations are performed on numeric values.

### Instructions

* [NOP](#nop-no-operation)
* [ADD](#add-adds-two-values)

#### NOP - no operation

Format | Description
-------|------------
0x00   | No operation

The *nop* instruction does nothing. It is intended to fill in space.

#### ADD - adds two values

Format | Description
-------|------------
0x01   | Adds two objects observing object addition rules.

The stack transitional behavior, in sequential order, is:
1. *value1* is pushed onto the stack
1. *value2* is pushed onto the stack
1. *value2* and *value1* are popped from stack; *value1* is added to *value2*
1. The result is pushed onto the stack

Addition rules:
1. If both *value1* and *value2* are `null`, then the result is `null`
1. If either *value1* or *value2* is of type `string` then string concatenation is performed (see [CONCAT](#concat-concatenates-two-values))
1. Otherwise arithmetic addition is performed (see [Arithmetic operations](#arithmetic-operations))
## Appendix
