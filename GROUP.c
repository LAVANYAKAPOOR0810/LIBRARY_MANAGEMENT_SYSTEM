#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

// Structures
typedef struct {
    char name[100];
    int quantity;
} Book;

typedef struct {
    char subject[100];
    Book books[10];
    int bookCount;
} Subject;

typedef struct {
    char streamName[100];
    Subject subjects[10];
    int subjectCount;
} Stream;

typedef struct {
    Stream streams[10];
    int streamCount;
} Library;

Library lib;

// Utility
void clearInput() {
    while (getchar() != '\n');
}

void waitForEnter() {
    printf("Press Enter to continue...");
    getchar();
}

// Get password with asterisks (works on Unix/Linux/macOS)
void getPassword(char* password, int maxLength) {
    struct termios oldt, newt;
    int i = 0;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt); // Get terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ECHO); // Turn off echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("Enter admin password: ");
    while ((ch = getchar()) != '\n' && i < maxLength - 1) {
        if (ch == 127 || ch == 8) { // Handle backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore terminal
    printf("\n");
}

// Book Functions
void addStream(const char* streamName) {
    strcpy(lib.streams[lib.streamCount].streamName, streamName);
    lib.streams[lib.streamCount].subjectCount = 0;
    lib.streamCount++;
}

void addSubjectToStream(Stream* stream, const char* subjectName) {
    strcpy(stream->subjects[stream->subjectCount].subject, subjectName);
    stream->subjects[stream->subjectCount].bookCount = 0;
    stream->subjectCount++;
}

void addBookToSubject(Subject* subject, const char* bookName, int quantity) {
    strcpy(subject->books[subject->bookCount].name, bookName);
    subject->books[subject->bookCount].quantity = quantity;
    subject->bookCount++;
}

// Load Data
void loadBooks() {
    lib.streamCount = 0;

    // BCA
    addStream("BCA");
    addSubjectToStream(&lib.streams[0], "Operating System");
    addBookToSubject(&lib.streams[0].subjects[0], "Modern OS by Tanenbaum", 10);
    addBookToSubject(&lib.streams[0].subjects[0], "Operating Systems by Galvin", 10);
    addSubjectToStream(&lib.streams[0], "Data Structures");
    addBookToSubject(&lib.streams[0].subjects[1], "Data Structures in C by Balagurusamy", 10);
    addBookToSubject(&lib.streams[0].subjects[1], "Data Structures Through C in Depth", 10);

    // MCA
    addStream("MCA");
    addSubjectToStream(&lib.streams[1], "DBMS");
    addBookToSubject(&lib.streams[1].subjects[0], "DBMS by Korth", 10);
    addBookToSubject(&lib.streams[1].subjects[0], "Database Concepts by Silberschatz", 10);
    addSubjectToStream(&lib.streams[1], "Networking");
    addBookToSubject(&lib.streams[1].subjects[1], "Computer Networks by Tanenbaum", 10);
    addBookToSubject(&lib.streams[1].subjects[1], "Data Communications by Forouzan", 10);

    // BTech
    addStream("BTech");
    addSubjectToStream(&lib.streams[2], "Machine Learning");
    addBookToSubject(&lib.streams[2].subjects[0], "Pattern Recognition by Bishop", 10);
    addBookToSubject(&lib.streams[2].subjects[0], "Hands-On ML by Aurélien Géron", 10);
    addSubjectToStream(&lib.streams[2], "AI");
    addBookToSubject(&lib.streams[2].subjects[1], "AI by Rich and Knight", 10);
    addBookToSubject(&lib.streams[2].subjects[1], "AI: A Modern Approach by Russell", 10);

    // BBA
    addStream("BBA");
    addSubjectToStream(&lib.streams[3], "Marketing");
    addBookToSubject(&lib.streams[3].subjects[0], "Marketing Management by Kotler", 10);
    addBookToSubject(&lib.streams[3].subjects[0], "Principles of Marketing", 10);
    addSubjectToStream(&lib.streams[3], "Finance");
    addBookToSubject(&lib.streams[3].subjects[1], "Financial Management by Khan", 10);
    addBookToSubject(&lib.streams[3].subjects[1], "Corporate Finance by Ross", 10);

    // BCom
    addStream("BCom");
    addSubjectToStream(&lib.streams[4], "Accounting");
    addBookToSubject(&lib.streams[4].subjects[0], "Financial Accounting by Tulsian", 10);
    addBookToSubject(&lib.streams[4].subjects[0], "Cost Accounting by Jain", 10);
    addSubjectToStream(&lib.streams[4], "Economics");
    addBookToSubject(&lib.streams[4].subjects[1], "Microeconomics by Mankiw", 10);
    addBookToSubject(&lib.streams[4].subjects[1], "Indian Economy by Dutt", 10);
}

// Display
void displayBooks() {
    printf("\n--- Library Book List ---\n");
    for (int i = 0; i < lib.streamCount; i++) {
        printf("\nStream: %s\n", lib.streams[i].streamName);
        for (int j = 0; j < lib.streams[i].subjectCount; j++) {
            printf("  Subject: %s\n", lib.streams[i].subjects[j].subject);
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                printf("    Book: %s | Quantity: %d\n",
                    lib.streams[i].subjects[j].books[k].name,
                    lib.streams[i].subjects[j].books[k].quantity);
            }
        }
    }
     waitForEnter();
}


void searchBook() {
    char keyword[100];
    int found = 0;
    clearInput();  // Clear input buffer
    printf("\nEnter book name to search: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0; // Remove trailing newline

    printf("\n--- Search Results ---\n");
    for (int i = 0; i < lib.streamCount; i++) {
        for (int j = 0; j < lib.streams[i].subjectCount; j++) {
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                if (strcasestr(lib.streams[i].subjects[j].books[k].name, keyword)) {
                    printf("Stream: %s | Subject: %s | Book: %s | Quantity: %d\n",
                           lib.streams[i].streamName,
                           lib.streams[i].subjects[j].subject,
                           lib.streams[i].subjects[j].books[k].name,
                           lib.streams[i].subjects[j].books[k].quantity);
                    found = 1;
                }
            }
        }
    }

    if (!found) {
        printf("No book found with the name '%s'.\n", keyword);
    }
    waitForEnter();
}


void filterBooksByStreamAndSubject() {
    int streamChoice, subjectChoice;

    printf("\n--- Filter: Stream and Subject ---\n");

    // List Streams
    for (int i = 0; i < lib.streamCount; i++) {
        printf("%d. %s\n", i + 1, lib.streams[i].streamName);
    }

    printf("Select a stream (1-%d): ", lib.streamCount);
    scanf("%d", &streamChoice);

    if (streamChoice < 1 || streamChoice > lib.streamCount) {
        printf("Invalid stream choice.\n");
        waitForEnter();
        return;
    }

    Stream* selectedStream = &lib.streams[streamChoice - 1];

    // List Subjects
    for (int j = 0; j < selectedStream->subjectCount; j++) {
        printf("%d. %s\n", j + 1, selectedStream->subjects[j].subject);
    }

    printf("Select a subject (1-%d): ", selectedStream->subjectCount);
    scanf("%d", &subjectChoice);

    if (subjectChoice < 1 || subjectChoice > selectedStream->subjectCount) {
        printf("Invalid subject choice.\n");
        waitForEnter();
        return;
    }

    Subject* selectedSubject = &selectedStream->subjects[subjectChoice - 1];

    printf("\n--- Books in %s > %s ---\n",
           selectedStream->streamName, selectedSubject->subject);

    for (int k = 0; k < selectedSubject->bookCount; k++) {
        printf("Book: %s | Quantity: %d\n",
               selectedSubject->books[k].name,
               selectedSubject->books[k].quantity);
    }

    waitForEnter();
}



// Menus
void adminMenu() {
    int choice;
    do {
        printf("\n--- Admin Menu ---\n");
        printf("1. View All Books\n2. Search Book by Name\n3. Filter by Stream & Subject\n4. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: filterBooksByStreamAndSubject(); break;
            case 4: printf("Logging out...\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 4);
}

void studentMenu() {
    int choice;
    do {
        printf("\n--- Student Menu ---\n");
        printf("1. View All Books\n2. Search Book\n3. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: printf("Logging out...\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 3);
}


int isStrongPassword(const char* password) {
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;

    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
        else if (ispunct(password[i])) hasSpecial = 1;
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}




// Login
void loginSystem() {
    int role;
    printf("\n--- Welcome to Library Management System ---\n");
    printf("Select role:\n1. Admin\n2. Student\nEnter choice: ");
    scanf("%d", &role);
    if (role == 1) {
        char pass[20];
        clearInput();
        getPassword(pass, sizeof(pass));
    if (strcmp(pass, "Admin@123") == 0) {
    if (!isStrongPassword(pass)) {
        printf("Weak password. Use at least 1 uppercase, 1 lowercase, 1 digit, and 1 special character.\n");
    } else {
        adminMenu();
    }
} else {
    printf("Incorrect password.\n");
}
    } else if (role == 2) {
        studentMenu();
    } else {
        printf("Invalid role.\n");
    }
}

// Main
int main() {
    loadBooks();
    while (1)
        loginSystem();
    return 0;
}
