# Boffer

A stand for practicing CFHjack in the Linux kernel

## Purpose

The purpose of this project is to provide a safe and controlled environment for learning about
Linux kernel security and exploiting vulnerabilities.

## Background

Control Flow Hjack is a well-known problem in Linux kernel security so there are various mitigations.
This project aims to provide a hands-on environment for learning about these topics and practicing exploitation techniques.

## Usage

```bash
python3 run.py
Usage: ./run.py [options]
Options:
  +smep    Enable SMEP (Supervisor Mode Execution Protection)
  -smep    Disable SMEP
  +smap    Enable SMAP (Supervisor Mode Access Protection)
  -smap    Disable SMAP
  --help   Display this help message and exit
```
