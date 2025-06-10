# 📬 PipeMail

**PipeMail** is a terminal-based interprocess communication (IPC) system that allows users to exchange messages using named pipes (FIFOs) and POSIX semaphores. Each user has a personal mailbox identified by their username, and messages are sent between these mailboxes.

---

## 🛠 Features

- 📮 User-specific mailboxes using FIFOs  
- 🔒 Semaphore-based access control  
- 📩 Send and receive text messages from the terminal  
- 🖥 Clean, color-coded CLI interface  
- 💥 Graceful shutdown on `Ctrl+C` with signal handling  

---

## 🚀 Getting Started

### Requirements

- Linux or Unix-based system  
- GCC  
- POSIX-compliant environment

### Build

```bash
gcc -o pipemail main.c ipc_utils.c -lrt -pthread
