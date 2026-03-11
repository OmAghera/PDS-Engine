# Persistent Data Engine (PDS) 💾

A custom, lightweight relational database engine built entirely from scratch in **C**. 

This project was developed iteratively across four versions, starting as a simple single-table generic data store and scaling into a dynamic system capable of handling an unlimited number of tables, strict one-to-one (1:1) entity relationships, and persistent file storage.

## 🚀 Project Evolution

I built this engine step-by-step to deeply understand low-level file I/O, memory management, and relational database architecture:
* **PDS 1.0:** Single-Table Generic Data Store.
* **PDS 2.0:** Two-Table Generic Data Store.
* **PDS 3.0:** Three-Table Generic Data Store (2 generic tables + 1 relationship table).
* **PDS 4.0:** Fully scaled engine supporting an multiple number of tables and relationships.

## ✨ Key Features (PDS 4.0)

* **Custom Schema Management:** Utilizes custom `.sch` files to define and store database schemas. The engine can seamlessly switch between multiple databases (handling one active database at a time).
* **Location-Based Indexing:** Implements custom indexing mechanics directly via C File I/O to optimize data retrieval and storage speeds without relying on external libraries.
* **Data Integrity & Soft Deletes:** Instead of permanent data erasure, the system utilizes a safe "soft delete" architecture by flagging records. 
* **Automatic Cascading Deletes:** Engineered a robust cascade mechanism that automatically identifies and soft-deletes all associated relational entries whenever a parent record is removed.
* **Complete CRUD Operations:** Full support for Create, Read, Update, and Delete operations across dynamically generated tables.

## ⚙️ System Constraints & Assumptions
To maintain strict structural integrity, this engine operates under the following design rules:
- Supports strict **one-to-one (1:1)** relationships between tables.
- Assuming globally unique table names and unique primary keys.
- Manages relations for a single active database at a time.

## 🛠️ Tech Stack
- **Language:** C
- **Core Concepts:** File I/O, Pointers, Memory Management, Custom Data Structures
