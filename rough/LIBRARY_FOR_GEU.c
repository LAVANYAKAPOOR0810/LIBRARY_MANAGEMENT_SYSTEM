#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#define MAX_STREAMS 5
#define MAX_SUBJECTS 5
#define MAX_BOOKS 10
#define PASSWORD_LENGTH 50
#define MAX_STUDENTS 100
#define MAX_ISSUED_BOOKS_PER_STUDENT 10
#define MAX_LOGS 500
#define BORROW_DAYS 14
#define FINE_PER_DAY 5  
#define TABLE_WIDTH 80

typedef struct {
    char name[100];
    int quantity;
} Book;

typedef struct {
    char subject[100];
    Book books[MAX_BOOKS];
    int bookCount;
} Subject;

typedef struct {
    char streamName[100];
    Subject subjects[MAX_SUBJECTS];
    int subjectCount;
} Stream;

typedef struct {
    int streamIndex;
    int subjectIndex;
    int bookIndex;
    char bookName[100];
    time_t issueDate;
    time_t dueDate;
    time_t returnDate;
    int isReturned;
} IssuedBook;

typedef struct {
    char username[50];
    char password[PASSWORD_LENGTH];
    IssuedBook issuedBooks[MAX_ISSUED_BOOKS_PER_STUDENT];
    int issuedBookCount;
} Student;

typedef struct {
    int studentIndex;
    char username[50];
    char bookName[100];
    char action[10];
    time_t timestamp;
} LogEntry;

typedef struct {
    Stream streams[MAX_STREAMS];
    int streamCount;
} Library;

Library lib;
int studentCount = 0;
Student students[MAX_STUDENTS];
LogEntry logs[MAX_LOGS];
int logCount = 0;

// Helper functions declarations
void clearInput();
void waitForEnter();
char *strcasestr_custom(const char *haystack, const char *needle);
void getPassword(char* password, int maxLength);
int isStrongPassword(const char* password);
void printDate(time_t t);
void printHeader(const char *title);
void printLine(int width);
void printCenteredLine(const char *text, int width);

// Student management
void saveStudents();
void loadStudents();
int usernameExists(const char* username);
int validateStudentLogin(const char* username, const char* password);
void signup();
int studentLogin();

// Book and Library functions
void loadBooks();
void displayBooks();
void searchBook();
void filterBooksByStreamAndSubject();
void issueBook(int loggedInStudentIndex);
void returnBook(int loggedInStudentIndex);
void showIssuedBooksByStudent(int studentIndex);
int calculateFine(time_t dueDate, time_t returnDate);

// Logs
void addLog(int studentIndex, const char* username, const char* bookName, const char* action);
void adminReportMenu();

// Menus
void adminMenu();
void studentMenu(int loggedInStudentIndex);
void loginSystem();

int main() {
    loadBooks();
    loadStudents();
    loginSystem();
    saveStudents();
    printf("\nThanks for using Library Management System!\n");
    return 0;
}

// --- Helper functions ---
void clearInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void waitForEnter() {
    printf("\nPress Enter to continue...");
    clearInput();
}

void printLine(int width) {
    for (int i = 0; i < width; i++) putchar('=');
    putchar('\n');
}

void printCenteredLine(const char *text, int width) {
    int len = (int)strlen(text);
    int leftPadding = (width - len) / 2;
    int rightPadding = width - len - leftPadding;
    for (int i = 0; i < leftPadding; i++) putchar(' ');
    printf("%s", text);
    for (int i = 0; i < rightPadding; i++) putchar(' ');
    putchar('\n');
}

void printHeader(const char *title) {
    system("clear");
    printLine(TABLE_WIDTH);
    printCenteredLine(title, TABLE_WIDTH);
    printLine(TABLE_WIDTH);
}

char *strcasestr_custom(const char *haystack, const char *needle) {
    if (!*needle) return (char *) haystack;
    for (; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; h++, n++) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n))
                    break;
            }
            if (!*n)
                return (char *) haystack;
        }
    }
    return NULL;
}

void getPassword(char* password, int maxLength) {
    struct termios oldt, newt;
    int i = 0;
    char c;

    // Turn off echo
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < maxLength - 1 && (c = getchar()) != '\n' && c != EOF) {
        if (c == 127 || c == 8) { // Backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            password[i++] = c;
            printf("*");
        }
    }
    password[i] = '\0';

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

int isStrongPassword(const char* password) {
    int hasUpper=0, hasLower=0, hasDigit=0, hasSpecial=0;
    if (strlen(password) < 8) return 0;
    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
        else hasSpecial = 1;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

void printDate(time_t t) {
    if (t == 0) {
        printf("N/A");
        return;
    }
    char buff[20];
    struct tm *tm_info = localtime(&t);
    strftime(buff, sizeof(buff), "%Y-%m-%d", tm_info);
    printf("%s", buff);
}

// --- Student data management ---
void saveStudents() {
    FILE *fp = fopen("students.dat", "wb");
    if (fp) {
        fwrite(&studentCount, sizeof(int), 1, fp);
        fwrite(students, sizeof(Student), studentCount, fp);
        fclose(fp);
    }
    // save logs as well
    fp = fopen("logs.dat", "wb");
    if (fp) {
        fwrite(&logCount, sizeof(int), 1, fp);
        fwrite(logs, sizeof(LogEntry), logCount, fp);
        fclose(fp);
    }
}

void loadStudents() {
    FILE *fp = fopen("students.dat", "rb");
    if (fp) {
        fread(&studentCount, sizeof(int), 1, fp);
        fread(students, sizeof(Student), studentCount, fp);
        fclose(fp);
    }
    fp = fopen("logs.dat", "rb");
    if (fp) {
        fread(&logCount, sizeof(int), 1, fp);
        fread(logs, sizeof(LogEntry), logCount, fp);
        fclose(fp);
    }
}

int usernameExists(const char* username) {
    for (int i = 0; i < studentCount; i++) {
        if (strcmp(students[i].username, username) == 0)
            return 1;
    }
    return 0;
}

int validateStudentLogin(const char* username, const char* password) {
    for (int i = 0; i < studentCount; i++) {
        if (strcmp(students[i].username, username) == 0 &&
            strcmp(students[i].password, password) == 0)
            return i; // return student index
    }
    return -1;
}

void signup() {
    char username[50], password[PASSWORD_LENGTH], passwordConfirm[PASSWORD_LENGTH];
    clearInput();
    printHeader("Student Signup");
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    if (usernameExists(username)) {
        printf("\n[!] Username already exists. Try logging in.\n");
        waitForEnter();
        return;
    }

    while (1) {
        printf("Enter password: ");
        getPassword(password, PASSWORD_LENGTH);
        if (!isStrongPassword(password)) {
            printf("[!] Password must be at least 8 characters long and include uppercase, lowercase, digit and special char.\n");
            continue;
        }
        printf("Confirm password: ");
        getPassword(passwordConfirm, PASSWORD_LENGTH);
        if (strcmp(password, passwordConfirm) != 0) {
            printf("[!] Passwords do not match. Try again.\n");
            continue;
        }
        break;
    }

    strcpy(students[studentCount].username, username);
    strcpy(students[studentCount].password, password);
    students[studentCount].issuedBookCount = 0;
    studentCount++;
    saveStudents();

    printf("\nSignup successful! You can now login.\n");
    waitForEnter();
}

int studentLogin() {
    char username[50], password[PASSWORD_LENGTH];
    clearInput();
    printHeader("Student Login");
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    getPassword(password, PASSWORD_LENGTH);

    int sIndex = validateStudentLogin(username, password);
    if (sIndex != -1) {
        printf("\nLogin successful! Welcome, %s\n", username);
        waitForEnter();
        return sIndex;
    } else {
        printf("\n[!] Invalid credentials.\n");
        waitForEnter();
        return -1;
    }
}

// --- Library books data ---
void loadBooks() {
    lib.streamCount = MAX_STREAMS;

    // BCA
    strcpy(lib.streams[0].streamName, "BCA");
    lib.streams[0].subjectCount = 2;
    strcpy(lib.streams[0].subjects[0].subject, "Data Structures");
    lib.streams[0].subjects[0].bookCount = 2;
    strcpy(lib.streams[0].subjects[0].books[0].name, "Data Structures in C");
    lib.streams[0].subjects[0].books[0].quantity = 5;
    strcpy(lib.streams[0].subjects[0].books[1].name, "Algorithms Unlocked");
    lib.streams[0].subjects[0].books[1].quantity = 3;
    strcpy(lib.streams[0].subjects[1].subject, "Database Management");
    lib.streams[0].subjects[1].bookCount = 1;
    strcpy(lib.streams[0].subjects[1].books[0].name, "Database System Concepts");
    lib.streams[0].subjects[1].books[0].quantity = 4;

    // MCA
    strcpy(lib.streams[1].streamName, "MCA");
    lib.streams[1].subjectCount = 2;
    strcpy(lib.streams[1].subjects[0].subject, "Operating Systems");
    lib.streams[1].subjects[0].bookCount = 2;
    strcpy(lib.streams[1].subjects[0].books[0].name, "Operating System Concepts");
    lib.streams[1].subjects[0].books[0].quantity = 6;
    strcpy(lib.streams[1].subjects[0].books[1].name, "Modern Operating Systems");
    lib.streams[1].subjects[0].books[1].quantity = 2;
    strcpy(lib.streams[1].subjects[1].subject, "Advanced Java");
    lib.streams[1].subjects[1].bookCount = 1;
    strcpy(lib.streams[1].subjects[1].books[0].name, "Java: The Complete Reference");
    lib.streams[1].subjects[1].books[0].quantity = 7;

    // BTech
    strcpy(lib.streams[2].streamName, "BTech");
    lib.streams[2].subjectCount = 2;
    strcpy(lib.streams[2].subjects[0].subject, "Computer Networks");
    lib.streams[2].subjects[0].bookCount = 2;
    strcpy(lib.streams[2].subjects[0].books[0].name, "Computer Networking");
    lib.streams[2].subjects[0].books[0].quantity = 4;
    strcpy(lib.streams[2].subjects[0].books[1].name, "Data Communication and Networking");
    lib.streams[2].subjects[0].books[1].quantity = 3;
    strcpy(lib.streams[2].subjects[1].subject, "Microprocessors");
    lib.streams[2].subjects[1].bookCount = 1;
    strcpy(lib.streams[2].subjects[1].books[0].name, "Microprocessor Architecture");
    lib.streams[2].subjects[1].books[0].quantity = 5;

    // BCom
    strcpy(lib.streams[3].streamName, "BCom");
    lib.streams[3].subjectCount = 1;
    strcpy(lib.streams[3].subjects[0].subject, "Accounting");
    lib.streams[3].subjects[0].bookCount = 2;
    strcpy(lib.streams[3].subjects[0].books[0].name, "Financial Accounting");
    lib.streams[3].subjects[0].books[0].quantity = 8;
    strcpy(lib.streams[3].subjects[0].books[1].name, "Cost Accounting");
    lib.streams[3].subjects[0].books[1].quantity = 4;

    // BBA
    strcpy(lib.streams[4].streamName, "BBA");
    lib.streams[4].subjectCount = 1;
    strcpy(lib.streams[4].subjects[0].subject, "Marketing");
    lib.streams[4].subjects[0].bookCount = 2;
    strcpy(lib.streams[4].subjects[0].books[0].name, "Principles of Marketing");
    lib.streams[4].subjects[0].books[0].quantity = 6;
    strcpy(lib.streams[4].subjects[0].books[1].name, "Consumer Behavior");
    lib.streams[4].subjects[0].books[1].quantity = 5;
}

void displayBooks() {
    printHeader("Available Books");
    printf("| %-12s | %-20s | %-35s | %8s |\n", "Stream", "Subject", "Book Name", "Quantity");
    printLine(TABLE_WIDTH);
    for (int i = 0; i < lib.streamCount; i++) {
        for (int j = 0; j < lib.streams[i].subjectCount; j++) {
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                printf("| %-12s | %-20s | %-35s | %8d |\n",
                    lib.streams[i].streamName,
                    lib.streams[i].subjects[j].subject,
                    lib.streams[i].subjects[j].books[k].name,
                    lib.streams[i].subjects[j].books[k].quantity);
            }
        }
    }
    printLine(TABLE_WIDTH);
    waitForEnter();
}

void searchBook() {
    char keyword[100];
    clearInput();
    printHeader("Search Book");
    printf("Enter book name keyword to search: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;

    int found = 0;
    printLine(TABLE_WIDTH);
    printf("| %-12s | %-20s | %-35s | %8s |\n", "Stream", "Subject", "Book Name", "Quantity");
    printLine(TABLE_WIDTH);
    for (int i = 0; i < lib.streamCount; i++) {
        for (int j = 0; j< lib.streams[i].subjectCount; j++) {
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                if (strcasestr_custom(lib.streams[i].subjects[j].books[k].name, keyword)) {
                    printf("| %-12s | %-20s | %-35s | %8d |\n",
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
        printf("\n[!] No books found matching '%s'\n", keyword);
    } else {
        printLine(TABLE_WIDTH);
    }
    waitForEnter();
}

void filterBooksByStreamAndSubject() {
    printHeader("Filter Books by Stream & Subject");
    printf("Available Streams:\n");
    for (int i = 0; i < lib.streamCount; i++) {
        printf("%d. %s\n", i+1, lib.streams[i].streamName);
    }
    printf("\nEnter stream number: ");
    int s;
    scanf("%d", &s);
    clearInput();
    if (s < 1 || s > lib.streamCount) {
        printf("\n[!] Invalid stream number.\n");
        waitForEnter();
        return;
    }
    s--;

    printf("\nSubjects in %s:\n", lib.streams[s].streamName);
    for (int j = 0; j < lib.streams[s].subjectCount; j++) {
        printf("%d. %s\n", j+1, lib.streams[s].subjects[j].subject);
    }
    printf("\nEnter subject number: ");
    int sub;
    scanf("%d", &sub);
    clearInput();
    if (sub < 1 || sub > lib.streams[s].subjectCount) {
        printf("\n[!] Invalid subject number.\n");
        waitForEnter();
        return;
    }
    sub--;

    printHeader("Filtered Books");
    printf("| %-4s | %-35s | %8s |\n", "No.", "Book Name", "Quantity");
    printLine(TABLE_WIDTH);
    for (int k = 0; k < lib.streams[s].subjects[sub].bookCount; k++) {
        printf("| %-4d | %-35s | %8d |\n", k+1,
            lib.streams[s].subjects[sub].books[k].name,
            lib.streams[s].subjects[sub].books[k].quantity);
    }
    printLine(TABLE_WIDTH);
    waitForEnter();
}

// Issue, Return and other functions unchanged (with small UI improvements) ...

void addLog(int studentIndex, const char* username, const char* bookName, const char* action) {
    if (logCount >= MAX_LOGS) return;
    logs[logCount].studentIndex = studentIndex;
    strncpy(logs[logCount].username, username, 49);
    logs[logCount].username[49] = '\0';
    strncpy(logs[logCount].bookName, bookName, 99);
    logs[logCount].bookName[99] = '\0';
    strncpy(logs[logCount].action, action, 9);
    logs[logCount].action[9] = '\0';
    logs[logCount].timestamp = time(NULL);
    logCount++;
}

int calculateFine(time_t dueDate, time_t returnDate) {
    if (returnDate <= dueDate)
        return 0;
    double secondsLate = difftime(returnDate, dueDate);
    int daysLate = (int)(secondsLate / (60*60*24));
    if (daysLate == 0) daysLate = 1;
    return daysLate * FINE_PER_DAY;
}

void issueBook(int loggedInStudentIndex) {
    Student *student = &students[loggedInStudentIndex];
    printHeader("Issue Book");

    if(student->issuedBookCount >= MAX_ISSUED_BOOKS_PER_STUDENT) {
        printf("[!] You have reached the maximum limit of issued books.\n");
        waitForEnter();
        return;
    }

    filterBooksByStreamAndSubject();

    printf("Enter stream number to issue book from: ");
    int s; scanf("%d", &s);
    clearInput();
    if (s < 1 || s > lib.streamCount) {
        printf("\n[!] Invalid stream number.\n");
        waitForEnter();
        return;
    }
    s--;

    printf("Enter subject number: ");
    int sub; scanf("%d", &sub);
    clearInput();
    if (sub < 1 || sub > lib.streams[s].subjectCount) {
        printf("\n[!] Invalid subject number.\n");
        waitForEnter();
        return;
    }
    sub--;

    printf("Enter book number: ");
    int b; scanf("%d", &b);
    clearInput();
    if (b < 1 || b > lib.streams[s].subjects[sub].bookCount) {
        printf("\n[!] Invalid book number.\n");
        waitForEnter();
        return;
    }
    b--;

    if (lib.streams[s].subjects[sub].books[b].quantity > 0) {
        lib.streams[s].subjects[sub].books[b].quantity--;

        IssuedBook *ib = &student->issuedBooks[student->issuedBookCount];
        ib->streamIndex = s;
        ib->subjectIndex = sub;
        ib->bookIndex = b;
        strcpy(ib->bookName, lib.streams[s].subjects[sub].books[b].name);
        ib->issueDate = time(NULL);
        ib->dueDate = ib->issueDate + BORROW_DAYS * 24 * 60 * 60;
        ib->returnDate = 0;
        ib->isReturned = 0;
        student->issuedBookCount++;

        addLog(loggedInStudentIndex, student->username, ib->bookName, "Issued");

        printf("\n[+] Book '%s' issued successfully!\n", ib->bookName);
        printf("Due date: ");
        printDate(ib->dueDate);
        printf("\n");
    } else {
        printf("\n[!] Sorry, book not available currently.\n");
    }
    waitForEnter();
}

void returnBook(int loggedInStudentIndex) {
    Student *student = &students[loggedInStudentIndex];
    printHeader("Return Book");

    if (student->issuedBookCount == 0) {
        printf("[!] You have no issued books to return.\n");
        waitForEnter();
        return;
    }

    printf("| %-4s | %-35s | %-12s |\n", "No.", "Book Name", "Due Date");
    printLine(TABLE_WIDTH);
    int hasIssued = 0;
    for (int i = 0; i < student->issuedBookCount; i++) {
        IssuedBook *ib = &student->issuedBooks[i];
        if (ib->isReturned == 0) {
            printf("| %-4d | %-35s | ", i+1, ib->bookName);
            printDate(ib->dueDate);
            printf(" |\n");
            hasIssued = 1;
        }
    }
    printLine(TABLE_WIDTH);
    if (!hasIssued) {
        printf("All books have been returned.\n");
        waitForEnter();
        return;
    }

    printf("Enter the number of the book to return: ");
    int choice;
    scanf("%d", &choice);
    clearInput();
    if (choice < 1 || choice > student->issuedBookCount || student->issuedBooks[choice -1].isReturned) {
        printf("[!] Invalid choice.\n");
        waitForEnter();
        return;
    }

    IssuedBook *ib = &student->issuedBooks[choice - 1];
    int s = ib->streamIndex;
    int sub = ib->subjectIndex;
    int b = ib->bookIndex;

    lib.streams[s].subjects[sub].books[b].quantity++;
    ib->isReturned = 1;
    ib->returnDate = time(NULL);

    int fine = calculateFine(ib->dueDate, ib->returnDate);
    if (fine > 0) {
        printf("\n[!] Book '%s' returned. You have a fine of %d units for late return.\n", ib->bookName, fine);
    } else {
        printf("\n[+] Book '%s' returned successfully, no fine.\n", ib->bookName);
    }

    addLog(loggedInStudentIndex, student->username, ib->bookName, "Returned");

    waitForEnter();
}

void showIssuedBooksByStudent(int studentIndex) {
    Student *student = &students[studentIndex];
    printHeader("My Issued Books");
    if (student->issuedBookCount == 0) {
        printf("You have no issued books.\n");
    } else {
        printf("| %-4s | %-35s | %-12s | %-12s | %-10s |\n", "No.", "Book Name", "Issued On", "Due Date", "Status");
        printLine(TABLE_WIDTH);
        for (int i = 0; i < student->issuedBookCount; i++) {
            IssuedBook *ib = &student->issuedBooks[i];
            printf("| %-4d | %-35s | ", i+1, ib->bookName);
            printDate(ib->issueDate);
            printf(" | ");
            printDate(ib->dueDate);
            printf(" | %-10s |\n", ib->isReturned ? "Returned" : "Issued");
        }
        printLine(TABLE_WIDTH);
    }
    waitForEnter();
}

void adminReportMenu() {
    while (1) {
        printHeader("Admin Report: Issued/Returned Books");
        if (logCount == 0) {
            printf("No records found.\n");
        } else {
            printf("| %-4s | %-15s | %-35s | %-10s | %-19s |\n", "No.", "User", "Book Name", "Action", "Date/Time");
            printLine(TABLE_WIDTH);
            for (int i = 0; i < logCount; i++) {
                char timebuff[20];
                struct tm *tm_info = localtime(&logs[i].timestamp);
                strftime(timebuff, 20, "%Y-%m-%d %H:%M:%S", tm_info);
                printf("| %-4d | %-15s | %-35s | %-10s | %-19s |\n",
                    i+1, logs[i].username, logs[i].bookName, logs[i].action, timebuff);
            }
            printLine(TABLE_WIDTH);
        }
        printf("Press Enter to return to Admin menu...");
        clearInput();
        getchar();
        return;
    }
}

void adminMenu() {
    while (1) {
        printHeader("Admin Menu");
        printf("1. View All Books\n");
        printf("2. Search Book\n");
        printf("3. Filter Books by Stream & Subject\n");
        printf("4. View Issued/Returned Logs\n");
        printf("5. Logout\n");
        printf("\nEnter choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: filterBooksByStreamAndSubject(); break;
            case 4: adminReportMenu(); break;
            case 5: return;
            default: printf("\n[!] Invalid choice\n"); waitForEnter();
        }
    }
}

void studentMenu(int loggedInStudentIndex) {
    while (1) {
        printHeader("Student Menu");
        printf("1. View All Books\n");
        printf("2. Search Book\n");
        printf("3. Filter Books by Stream & Subject\n");
        printf("4. Issue Book\n");
        printf("5. Return Book\n");
        printf("6. View My Issued Books\n");
        printf("7. Logout\n");
        printf("\nEnter choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: filterBooksByStreamAndSubject(); break;
            case 4: issueBook(loggedInStudentIndex); break;
            case 5: returnBook(loggedInStudentIndex); break;
            case 6: showIssuedBooksByStudent(loggedInStudentIndex); break;
            case 7: return;
            default: printf("\n[!] Invalid choice\n"); waitForEnter();
        }
    }
}

void loginSystem() {
    while (1) {
        printHeader("Library Management System");
        printf("1. Admin Login\n");
        printf("2. Student Login\n");
        printf("3. Student Signup\n");
        printf("4. Exit\n");
        printf("\nEnter choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        if(choice == 1){
            char adminUser[50], adminPass[PASSWORD_LENGTH];
            printHeader("Admin Login");
            printf("Admin username: ");
            fgets(adminUser, sizeof(adminUser), stdin);
            adminUser[strcspn(adminUser, "\n")] = 0;

            printf("Admin password: ");
            getPassword(adminPass, PASSWORD_LENGTH);

            if (strcmp(adminUser, "admin") == 0 && strcmp(adminPass, "Admin@123") == 0) {
                printf("\nAdmin login successful.\n");
                waitForEnter();
                adminMenu();
            } else {
                printf("\n[!] Invalid admin credentials.\n");
                waitForEnter();
            }
        }
        else if(choice == 2){
            int sIndex = studentLogin();
            if(sIndex != -1)
                studentMenu(sIndex);
        }
        else if(choice == 3){
            signup();
        }
        else if(choice == 4){
            break;
        }
        else{
            printf("\n[!] Invalid choice.\n");
            waitForEnter();
        }
    }
}