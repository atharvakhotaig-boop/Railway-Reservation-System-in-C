# Railway Reservation System (C - DSA Project)

This project is a terminal-based Railway Reservation System implemented in C. It demonstrates the application of core Data Structures and Algorithms (DSA) concepts in a real-world system, focusing on efficient data management and clean program design.

---

## Features

* Add and manage multiple trains
* Book tickets with automatic seat allocation
* Cancel tickets with dynamic seat reassignment
* Waitlist management using a queue
* Reservation handling using a circular linked list
* Display reservations for each train
* Terminal-based user interface with colored output
* Robust input handling to prevent crashes and invalid states

---

## Data Structures Used

* Doubly Linked List
  Used for managing multiple trains efficiently with forward and backward traversal

* Circular Linked List
  Used for storing passenger reservations to enable continuous traversal without null checks

* Queue (Linked List Implementation)
  Used for managing the waitlist in a FIFO manner

---

## System Workflow

Booking:

* If seats are available, the passenger is added to the reservation list
* If seats are full, the passenger is added to the waitlist

Cancellation:

* The passenger is removed from the reservation list
* Seat availability is updated
* The first passenger from the waitlist is moved to the reservation list automatically

---

## Technologies Used

* Programming Language: C
* Concepts: Data Structures (Linked Lists, Queue)
* Interface: Command Line Interface (CLI)

---

## Compilation and Execution

Compile the program:

```bash
gcc main.c -o railway
```

Run the program:

```bash
./railway
```

---

## Project Structure

```
main.c   Contains complete implementation including data structures, logic, and user interface
```

---

## Input Handling

* The program uses safe input methods to avoid issues with invalid or mixed input
* Only valid numeric input is accepted where required
* Incorrect input is handled gracefully without crashing the program

---

## Future Enhancements

* Hash table for constant time passenger search
* Priority queue for priority-based booking (Tatkal or VIP)
* File handling to persist data across executions
* Seat map visualization in terminal
* Graph-based route optimization using shortest path algorithms

---

## Learning Outcomes

* Implementation of multiple data structures in a single integrated system
* Understanding of real-world problem modeling using DSA
* Handling edge cases and user input validation in C
* Writing structured, efficient, and maintainable code

---

## Author

Atharva Khot
BTech Computer Engineering

---

## License

This project is for academic and learning purposes.
