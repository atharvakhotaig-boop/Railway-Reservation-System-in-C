#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define COLOR_ORANGE "\033[38;5;208m"
#define COLOR_WHITE  "\033[97m"
#define COLOR_GREEN  "\033[92m"
#define COLOR_RED    "\033[91m"
#define COLOR_RESET  "\033[0m"

// ─────────────────────────────────────────
//  Safe input helpers
// ─────────────────────────────────────────
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ─────────────────────────────────────────
//  Structs
// ─────────────────────────────────────────
typedef struct Passenger {
    int  id;
    char name[MAX_NAME];
    int  age;
    int  seat;
    struct Passenger *next;
} Passenger;

typedef struct Queue {
    Passenger      *data;
    struct Queue   *next;
} Queue;

typedef struct Train {
    int    train_id;
    char   name[MAX_NAME];
    int    total_seats;
    int    available_seats;
    Passenger     *reservations;
    Queue         *wait_front;
    Queue         *wait_rear;
    struct Train  *next;
    struct Train  *prev;
} Train;

Train *head          = NULL;
int    passengerCounter = 1;

// ─────────────────────────────────────────
//  UI helpers
// ─────────────────────────────────────────
void header(const char *title) {
    printf(COLOR_ORANGE "\n===== %s =====\n" COLOR_RESET, title);
}
void success(const char *msg) {
    printf(COLOR_GREEN  "%s\n" COLOR_RESET, msg);
}
void error(const char *msg) {
    printf(COLOR_RED    "%s\n" COLOR_RESET, msg);
}

// ─────────────────────────────────────────
//  Train – doubly linked list
// ─────────────────────────────────────────
Train *createTrain(int id, char name[], int seats) {
    Train *t = (Train *)malloc(sizeof(Train));
    if (!t) { error("Memory allocation failed."); return NULL; }
    t->train_id        = id;
    strncpy(t->name, name, MAX_NAME - 1);
    t->name[MAX_NAME - 1] = '\0';
    t->total_seats     = seats;
    t->available_seats = seats;
    t->reservations    = NULL;
    t->wait_front = t->wait_rear = NULL;
    t->next = t->prev = NULL;
    return t;
}

void addTrain() {
    int  id, seats;
    char name[MAX_NAME];

    header("Add Train");
    printf("Enter Train ID   : "); scanf("%d", &id); clearInputBuffer();
    printf("Enter Train Name : "); fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = '\0';
    printf("Total Seats      : "); scanf("%d", &seats); clearInputBuffer();

    // Duplicate ID check
    Train *temp = head;
    while (temp) {
        if (temp->train_id == id) { error("Train ID already exists."); return; }
        temp = temp->next;
    }

    Train *t = createTrain(id, name, seats);
    if (!t) return;

    if (!head) {
        head = t;
    } else {
        temp = head;
        while (temp->next) temp = temp->next;
        temp->next = t;
        t->prev    = temp;
    }
    success("Train Added Successfully.");
}

Train *findTrain(int id) {
    Train *temp = head;
    while (temp) {
        if (temp->train_id == id) return temp;
        temp = temp->next;
    }
    return NULL;
}

void displayTrains() {
    header("All Trains");
    if (!head) { error("No trains available."); return; }

    Train *temp = head;
    while (temp) {
        printf(COLOR_WHITE
               "  ID: %d | Name: %-20s | Seats: %d/%d | Waitlisted: ",
               temp->train_id, temp->name,
               temp->available_seats, temp->total_seats);

        // Count waitlisted passengers
        int wc = 0;
        Queue *q = temp->wait_front;
        while (q) { wc++; q = q->next; }
        printf("%d\n" COLOR_RESET, wc);

        temp = temp->next;
    }
}

void deleteTrain() {
    int id;
    header("Delete Train");
    printf("Enter Train ID to delete: "); scanf("%d", &id); clearInputBuffer();

    Train *t = findTrain(id);
    if (!t) { error("Train not found."); return; }

    // Free reservations
    Passenger *p = t->reservations;
    while (p) { Passenger *tmp = p->next; free(p); p = tmp; }

    // Free waitlist
    Queue *q = t->wait_front;
    while (q) {
        Queue *tmp = q->next;
        free(q->data);
        free(q);
        q = tmp;
    }

    // Relink doubly linked list
    if (t->prev) t->prev->next = t->next;
    else         head          = t->next;
    if (t->next) t->next->prev = t->prev;

    free(t);
    success("Train Deleted Successfully.");
}

// ─────────────────────────────────────────
//  Passenger linked list helpers
// ─────────────────────────────────────────
Passenger *createPassenger(char name[], int age) {
    Passenger *p = (Passenger *)malloc(sizeof(Passenger));
    if (!p) { error("Memory allocation failed."); return NULL; }
    p->id   = passengerCounter++;
    strncpy(p->name, name, MAX_NAME - 1);
    p->name[MAX_NAME - 1] = '\0';
    p->age  = age;
    p->seat = 0;       // 0 = waitlisted
    p->next = NULL;
    return p;
}

// Assign the next available seat number (1-based)
int assignSeat(Train *t) {
    // Find lowest seat number not already taken
    for (int s = 1; s <= t->total_seats; s++) {
        int taken = 0;
        Passenger *p = t->reservations;
        while (p) {
            if (p->seat == s) { taken = 1; break; }
            p = p->next;
        }
        if (!taken) return s;
    }
    return -1; // no seat free (shouldn't happen if available_seats > 0)
}

// ─────────────────────────────────────────
//  Waitlist queue
// ─────────────────────────────────────────
void enqueue(Train *t, Passenger *p) {
    Queue *node = (Queue *)malloc(sizeof(Queue));
    if (!node) { error("Memory allocation failed."); return; }
    node->data = p;
    node->next = NULL;
    if (!t->wait_rear) {
        t->wait_front = t->wait_rear = node;
    } else {
        t->wait_rear->next = node;
        t->wait_rear       = node;
    }
}

// Returns passenger at front of queue (removes node, keeps passenger)
Passenger *dequeue(Train *t) {
    if (!t->wait_front) return NULL;
    Queue     *node = t->wait_front;
    Passenger *p    = node->data;
    t->wait_front   = node->next;
    if (!t->wait_front) t->wait_rear = NULL;
    free(node);
    return p;
}

// ─────────────────────────────────────────
//  Reservation
// ─────────────────────────────────────────
void bookTicket() {
    int  trainId, age;
    char name[MAX_NAME];

    header("Book Ticket");
    printf("Enter Train ID   : "); scanf("%d", &trainId); clearInputBuffer();

    Train *t = findTrain(trainId);
    if (!t) { error("Train not found."); return; }

    printf("Passenger Name   : "); fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = '\0';
    printf("Passenger Age    : "); scanf("%d", &age); clearInputBuffer();

    Passenger *p = createPassenger(name, age);
    if (!p) return;

    if (t->available_seats > 0) {
        p->seat = assignSeat(t);
        t->available_seats--;

        // Append to reservations list
        if (!t->reservations) {
            t->reservations = p;
        } else {
            Passenger *tmp = t->reservations;
            while (tmp->next) tmp = tmp->next;
            tmp->next = p;
        }
        printf(COLOR_GREEN
               "Booking Confirmed! Passenger ID: %d | Seat: %d\n"
               COLOR_RESET, p->id, p->seat);
    } else {
        enqueue(t, p);
        printf(COLOR_WHITE
               "No seats available. Added to Waitlist. Passenger ID: %d\n"
               COLOR_RESET, p->id);
    }
}

void cancelTicket() {
    int trainId, passId;

    header("Cancel Ticket");
    printf("Enter Train ID     : "); scanf("%d", &trainId); clearInputBuffer();
    printf("Enter Passenger ID : "); scanf("%d", &passId);  clearInputBuffer();

    Train *t = findTrain(trainId);
    if (!t) { error("Train not found."); return; }

    // Search in reservations
    Passenger *prev = NULL, *cur = t->reservations;
    while (cur) {
        if (cur->id == passId) break;
        prev = cur;
        cur  = cur->next;
    }

    if (!cur) {
        error("Passenger not found in reservations (check waitlist separately).");
        return;
    }

    // Remove from list
    if (prev) prev->next = cur->next;
    else       t->reservations = cur->next;

    printf(COLOR_WHITE "Cancelled: %s (Seat %d)\n" COLOR_RESET,
           cur->name, cur->seat);
    free(cur);
    t->available_seats++;

    // Promote from waitlist if anyone waiting
    Passenger *next_p = dequeue(t);
    if (next_p) {
        next_p->seat = assignSeat(t);
        t->available_seats--;

        // Append to reservations
        if (!t->reservations) {
            t->reservations = next_p;
        } else {
            Passenger *tmp = t->reservations;
            while (tmp->next) tmp = tmp->next;
            tmp->next = next_p;
        }
        printf(COLOR_GREEN
               "Waitlisted passenger promoted! %s → Seat %d (ID: %d)\n"
               COLOR_RESET, next_p->name, next_p->seat, next_p->id);
    }
    success("Cancellation Complete.");
}

// ─────────────────────────────────────────
//  Display
// ─────────────────────────────────────────
void displayPassengers() {
    int trainId;
    header("Passenger List");
    printf("Enter Train ID: "); scanf("%d", &trainId); clearInputBuffer();

    Train *t = findTrain(trainId);
    if (!t) { error("Train not found."); return; }

    printf(COLOR_ORANGE "Train: %s (ID %d)\n" COLOR_RESET, t->name, t->train_id);

    printf(COLOR_WHITE "  -- Confirmed Passengers --\n" COLOR_RESET);
    Passenger *p = t->reservations;
    if (!p) printf("  (none)\n");
    while (p) {
        printf("  [ID:%d] %-20s Age:%-3d Seat:%d\n",
               p->id, p->name, p->age, p->seat);
        p = p->next;
    }

    printf(COLOR_WHITE "  -- Waitlist --\n" COLOR_RESET);
    Queue *q = t->wait_front;
    int pos  = 1;
    if (!q) printf("  (none)\n");
    while (q) {
        printf("  #%d [ID:%d] %-20s Age:%d\n",
               pos++, q->data->id, q->data->name, q->data->age);
        q = q->next;
    }
}

void searchPassenger() {
    int passId;
    header("Search Passenger");
    printf("Enter Passenger ID: "); scanf("%d", &passId); clearInputBuffer();

    Train *t = head;
    while (t) {
        Passenger *p = t->reservations;
        while (p) {
            if (p->id == passId) {
                printf(COLOR_GREEN
                       "Found | Train: %s | Name: %s | Age: %d | Seat: %d\n"
                       COLOR_RESET,
                       t->name, p->name, p->age, p->seat);
                return;
            }
            p = p->next;
        }
        // Check waitlist too
        Queue *q = t->wait_front;
        while (q) {
            if (q->data->id == passId) {
                printf(COLOR_WHITE
                       "Found (Waitlist) | Train: %s | Name: %s | Age: %d\n"
                       COLOR_RESET,
                       t->name, q->data->name, q->data->age);
                return;
            }
            q = q->next;
        }
        t = t->next;
    }
    error("Passenger not found.");
}

// ─────────────────────────────────────────
//  Cleanup on exit
// ─────────────────────────────────────────
void freeAll() {
    Train *t = head;
    while (t) {
        Passenger *p = t->reservations;
        while (p) { Passenger *tmp = p->next; free(p); p = tmp; }

        Queue *q = t->wait_front;
        while (q) { Queue *tmp = q->next; free(q->data); free(q); q = tmp; }

        Train *tmp = t->next;
        free(t);
        t = tmp;
    }
}

// ─────────────────────────────────────────
//  Main menu
// ─────────────────────────────────────────
int main() {
    int choice;
    do {
        header("Railway Reservation System");
        printf(COLOR_WHITE
               "  1. Add Train\n"
               "  2. Delete Train\n"
               "  3. View All Trains\n"
               "  4. Book Ticket\n"
               "  5. Cancel Ticket\n"
               "  6. View Passengers\n"
               "  7. Search Passenger\n"
               "  0. Exit\n"
               COLOR_RESET);
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) { clearInputBuffer(); continue; }
        clearInputBuffer();

        switch (choice) {
            case 1: addTrain();         break;
            case 2: deleteTrain();      break;
            case 3: displayTrains();    break;
            case 4: bookTicket();       break;
            case 5: cancelTicket();     break;
            case 6: displayPassengers();break;
            case 7: searchPassenger();  break;
            case 0: success("Goodbye!"); break;
            default: error("Invalid option.");
        }
    } while (choice != 0);

    freeAll();
    return 0;
}