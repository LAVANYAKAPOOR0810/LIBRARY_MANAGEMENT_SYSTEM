
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define MAX_STREAMS 5
#define MAX_SUBJECTS 5
#define MAX_BOOKS 10
#define PASSWORD_LENGTH 50
#define MAX_STUDENTS 100

typedef struct {
    char name[100];
    char author[100]; // Added author field
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
    char username[50];
    char password[PASSWORD_LENGTH];
} Student;

typedef struct {
    Stream streams[MAX_STREAMS];
    int streamCount;
} Library;

Library lib;
int studentCount = 0;
Student students[MAX_STUDENTS];

// Helper functions declarations
void clearInput();
void waitForEnter();
char *strcasestr_custom(const char *haystack, const char *needle);
void getPassword(char* password, int maxLength);
int isStrongPassword(const char* password);
void printSeparator(char c, int length);

// Student management
void saveStudents();
void loadStudents();
int usernameExists(const char* username);
int validateStudentLogin(const char* username, const char* password);
void signup();
int studentLogin();

// Book and Library functions
void loadBooks();
void displayBooks(); // This will be enhanced
void searchBook();
void filterBooksByStreamAndSubject();
void issueBook();
void returnBook();

// Menus
void adminMenu();
void studentMenu();
void loginSystem();

int main() {
    loadBooks();
    loadStudents();
    loginSystem();
    saveStudents();
    printf("\n\n--- Thank you for using the Library Management System! ---\n\n");
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

void printSeparator(char c, int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", c);
    }
    printf("\n");
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

// --- Student data management ---
void saveStudents() {
    FILE *fp = fopen("students.dat", "wb");
    if (fp) {
        fwrite(&studentCount, sizeof(int), 1, fp);
        fwrite(students, sizeof(Student), studentCount, fp);
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
            return 1;
    }
    return 0;
}

void signup() {
    char username[50], password[PASSWORD_LENGTH], passwordConfirm[PASSWORD_LENGTH];
    clearInput();
    system("clear");
    printf("\n=============================\n");
    printf("     STUDENT SIGNUP          \n");
    printf("=============================\n\n");
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    if (usernameExists(username)) {
        printf("\n>>> ERROR: Username '%s' already exists. Please try logging in or choose another username.\n", username);
        waitForEnter();
        return;
    }

    while (1) {
        printf("Enter password (Min 8 chars, incl. upper, lower, digit, special): ");
        getPassword(password, PASSWORD_LENGTH);
        if (!isStrongPassword(password)) {
            printf(">>> Password does not meet strength requirements. Please try again.\n");
            continue;
        }
        printf("Confirm password: ");
        getPassword(passwordConfirm, PASSWORD_LENGTH);
        if (strcmp(password, passwordConfirm) != 0) {
            printf(">>> Passwords do not match. Please try again.\n");
            continue;
        }
        break;
    }

    strcpy(students[studentCount].username, username);
    strcpy(students[studentCount].password, password);
    studentCount++;
    saveStudents();

    printf("\n>>> Signup successful! You can now login with your new account.\n");
    waitForEnter();
}

int studentLogin() {
    char username[50], password[PASSWORD_LENGTH];
    clearInput();
    system("clear");
    printf("\n=============================\n");
    printf("      STUDENT LOGIN          \n");
    printf("=============================\n\n");
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    getPassword(password, PASSWORD_LENGTH);

    if (validateStudentLogin(username, password)) {
        printf("\n>>> Login successful! Welcome, %s.\n", username);
        waitForEnter();
        return 1;
    } else {
        printf("\n>>> ERROR: Invalid username or password. Please try again.\n");
        waitForEnter();
        return 0;
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
    strcpy(lib.streams[0].subjects[0].books[0].author, "Horowitz and Sahni"); // Added author
    lib.streams[0].subjects[0].books[0].quantity = 5;
    strcpy(lib.streams[0].subjects[0].books[1].name, "Algorithms Unlocked");
    strcpy(lib.streams[0].subjects[0].books[1].author, "Thomas H. Cormen"); // Added author
    lib.streams[0].subjects[0].books[1].quantity = 3;
    strcpy(lib.streams[0].subjects[1].subject, "Database Management");
    lib.streams[0].subjects[1].bookCount = 1;
    strcpy(lib.streams[0].subjects[1].books[0].name, "Database System Concepts");
    strcpy(lib.streams[0].subjects[1].books[0].author, "Silberschatz, Korth, Sudarshan"); // Added author
    lib.streams[0].subjects[1].books[0].quantity = 4;

    // MCA
    strcpy(lib.streams[1].streamName, "MCA");
    lib.streams[1].subjectCount = 2;
    strcpy(lib.streams[1].subjects[0].subject, "Operating Systems");
    lib.streams[1].subjects[0].bookCount = 2;
    strcpy(lib.streams[1].subjects[0].books[0].name, "Operating System Concepts");
    strcpy(lib.streams[1].subjects[0].books[0].author, "Silberschatz, Galvin, Gagne"); // Added author
    lib.streams[1].subjects[0].books[0].quantity = 6;
    strcpy(lib.streams[1].subjects[0].books[1].name, "Modern Operating Systems");
    strcpy(lib.streams[1].subjects[0].books[1].author, "Andrew S. Tanenbaum"); // Added author
    lib.streams[1].subjects[0].books[1].quantity = 2;
    strcpy(lib.streams[1].subjects[1].subject, "Advanced Java");
    lib.streams[1].subjects[1].bookCount = 1;
    strcpy(lib.streams[1].subjects[1].books[0].name, "Java: The Complete Reference");
    strcpy(lib.streams[1].subjects[1].books[0].author, "Herbert Schildt"); // Added author
    lib.streams[1].subjects[1].books[0].quantity = 7;

    // BTech
    strcpy(lib.streams[2].streamName, "BTech");
    lib.streams[2].subjectCount = 2;
    strcpy(lib.streams[2].subjects[0].subject, "Computer Networks");
    lib.streams[2].subjects[0].bookCount = 2;
    strcpy(lib.streams[2].subjects[0].books[0].name, "Computer Networking: A Top-Down Approach");
    strcpy(lib.streams[2].subjects[0].books[0].author, "James Kurose, Keith Ross"); // Added author
    lib.streams[2].subjects[0].books[0].quantity = 4;
    strcpy(lib.streams[2].subjects[0].books[1].name, "Data Communication and Networking");
    strcpy(lib.streams[2].subjects[0].books[1].author, "Behrouz Forouzan"); // Added author
    lib.streams[2].subjects[0].books[1].quantity = 3;
    strcpy(lib.streams[2].subjects[1].subject, "Microprocessors");
    lib.streams[2].subjects[1].bookCount = 1;
    strcpy(lib.streams[2].subjects[1].books[0].name, "Microprocessor Architecture");
    strcpy(lib.streams[2].subjects[1].books[0].author, "Ramesh Gaonkar"); // Added author
    lib.streams[2].subjects[1].books[0].quantity = 5;

    // BCom
    strcpy(lib.streams[3].streamName, "BCom");
    lib.streams[3].subjectCount = 1;
    strcpy(lib.streams[3].subjects[0].subject, "Accounting");
    lib.streams[3].subjects[0].bookCount = 2;
    strcpy(lib.streams[3].subjects[0].books[0].name, "Financial Accounting");
    strcpy(lib.streams[3].subjects[0].books[0].author, "P. C. Tulsian"); // Added author
    lib.streams[3].subjects[0].books[0].quantity = 8;
    strcpy(lib.streams[3].subjects[0].books[1].name, "Cost Accounting");
    strcpy(lib.streams[3].subjects[0].books[1].author, "M. N. Arora"); // Added author
    lib.streams[3].subjects[0].books[1].quantity = 4;

    // BBA
    strcpy(lib.streams[4].streamName, "BBA");
    lib.streams[4].subjectCount = 1;
    strcpy(lib.streams[4].subjects[0].subject, "Marketing");
    lib.streams[4].subjects[0].bookCount = 2;
    strcpy(lib.streams[4].subjects[0].books[0].name, "Principles of Marketing");
    strcpy(lib.streams[4].subjects[0].books[0].author, "Philip Kotler"); // Added author
    lib.streams[4].subjects[0].books[0].quantity = 6;
    strcpy(lib.streams[4].subjects[0].books[1].name, "Consumer Behavior");
    strcpy(lib.streams[4].subjects[0].books[1].author, "Leon G. Schiffman"); // Added author
    lib.streams[4].subjects[0].books[1].quantity = 5;
}

void displayBooks() {
    system("clear");
    printf("\n=================================================================================================\n");
    printf("                                         AVAILABLE BOOKS                                         \n");
    printf("=================================================================================================\n");
    if (lib.streamCount == 0) {
        printf("\nNo books currently available in the library.\n");
        waitForEnter();
        return;
    }

    for (int i = 0; i < lib.streamCount; i++) {
        printSeparator('=', 95);
        printf("  STREAM: %s\n", lib.streams[i].streamName);
        printSeparator('=', 95);
        if (lib.streams[i].subjectCount == 0) {
            printf("    No subjects available for this stream.\n");
            continue;
        }
        for (int j = 0; j < lib.streams[i].subjectCount; j++) {
            printf("    SUBJECT: %s\n", lib.streams[i].subjects[j].subject);
            printSeparator('-', 95);
            printf("    %-45s %-30s %-10s\n", "BOOK NAME", "AUTHOR", "QUANTITY");
            printSeparator('-', 95);
            if (lib.streams[i].subjects[j].bookCount == 0) {
                printf("    No books available for this subject.\n");
                continue;
            }
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                printf("    %-45s %-30s %-10d\n",
                    lib.streams[i].subjects[j].books[k].name,
                    lib.streams[i].subjects[j].books[k].author,
                    lib.streams[i].subjects[j].books[k].quantity);
            }
            printSeparator('-', 95);
        }
    }
    printf("\n");
    waitForEnter();
}

void searchBook() {
    char keyword[100];
    clearInput();
    system("clear");
    printf("\n=====================================\n");
    printf("           SEARCH BOOKS              \n");
    printf("=====================================\n\n");
    printf("Enter book name (or part of it) to search: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;

    int found = 0;
    printf("\nSearch results for '%s':\n", keyword);
    printSeparator('-', 95);
    printf("%-5s %-45s %-30s %-10s\n", "S.No.", "BOOK NAME", "AUTHOR", "QUANTITY");
    printSeparator('-', 95);
    int serialNum = 1;
    for (int i = 0; i < lib.streamCount; i++) {
        for (int j = 0; j< lib.streams[i].subjectCount; j++) {
            for (int k = 0; k < lib.streams[i].subjects[j].bookCount; k++) {
                if (strcasestr_custom(lib.streams[i].subjects[j].books[k].name, keyword) ||
                    strcasestr_custom(lib.streams[i].subjects[j].books[k].author, keyword)) { // Also search by author
                    printf("%-5d %-45s %-30s %-10d\n", serialNum++,
                        lib.streams[i].subjects[j].books[k].name,
                        lib.streams[i].subjects[j].books[k].author,
                        lib.streams[i].subjects[j].books[k].quantity);
                    found = 1;
                }
            }
        }
    }
    printSeparator('-', 95);
    if (!found) {
        printf("\n>>> No books or authors found matching '%s' in the library.\n", keyword);
    }
    printf("\n");
    waitForEnter();
}


void filterBooksByStreamAndSubject() {
    system("clear");
    printf("\n========================================\n");
    printf("   FILTER BOOKS BY STREAM AND SUBJECT   \n");
    printf("========================================\n\n");

    printf("Available Streams:\n");
    printSeparator('-', 30);
    for (int i = 0; i < lib.streamCount; i++) {
        printf("%2d. %s\n", i+1, lib.streams[i].streamName);
    }
    printSeparator('-', 30);
    printf("Enter stream number: ");
    int s;
    scanf("%d", &s);
    clearInput();
    if (s < 1 || s > lib.streamCount) {
        printf(">>> Invalid stream number. Please try again.\n");
        waitForEnter();
        return;
    }
    s--;

    printf("\nSubjects in %s:\n", lib.streams[s].streamName);
    printSeparator('-', 30);
    for (int j = 0; j < lib.streams[s].subjectCount; j++) {
        printf("%2d. %s\n", j+1, lib.streams[s].subjects[j].subject);
    }
    printSeparator('-', 30);
    printf("Enter subject number: ");
    int sub;
    scanf("%d", &sub);
    clearInput();
    if (sub < 1 || sub > lib.streams[s].subjectCount) {
        printf(">>> Invalid subject number. Please try again.\n");
        waitForEnter();
        return;
    }
    sub--;

    printf("\nBooks in %s -> %s:\n", lib.streams[s].streamName, lib.streams[s].subjects[sub].subject);
    printSeparator('=', 95);
    printf("%-5s %-45s %-30s %-10s\n", "S.No.", "BOOK NAME", "AUTHOR", "QUANTITY");
    printSeparator('=', 95);
    if (lib.streams[s].subjects[sub].bookCount == 0) {
        printf("    No books available in this subject.\n");
    } else {
        for (int k = 0; k < lib.streams[s].subjects[sub].bookCount; k++) {
            printf("%-5d %-45s %-30s %-10d\n", k+1,
                lib.streams[s].subjects[sub].books[k].name,
                lib.streams[s].subjects[sub].books[k].author,
                lib.streams[s].subjects[sub].books[k].quantity);
        }
    }
    printSeparator('=', 95);
    printf("\n");
    waitForEnter();
}

// issueBook and returnBook remain unchanged since students still use them

void issueBook() {
    system("clear");
    printf("\n=============================\n");
    printf("         ISSUE BOOK          \n");
    printf("=============================\n");
    // Reuse filtering for selection but display in tabular format for clarity
    printf("\nTo issue a book, first select the stream and subject.\n");
    filterBooksByStreamAndSubject(); 

    printf("\nEnter stream number to issue book from: ");
    int s; scanf("%d", &s);
    clearInput();
    if (s < 1 || s > lib.streamCount) {
        printf(">>> Invalid stream number. Please try again.\n");
        waitForEnter();
        return;
    }
    s--;

    printf("Enter subject number: ");
    int sub; scanf("%d", &sub);
    clearInput();
    if (sub < 1 || sub > lib.streams[s].subjectCount) {
        printf(">>> Invalid subject number. Please try again.\n");
        waitForEnter();
        return;
    }
    sub--;

    printf("Enter book number: ");
    int b; scanf("%d", &b);
    clearInput();
    if (b < 1 || b > lib.streams[s].subjects[sub].bookCount) {
        printf(">>> Invalid book number. Please try again.\n");
        waitForEnter();
        return;
    }
    b--;

    if (lib.streams[s].subjects[sub].books[b].quantity > 0) {
        lib.streams[s].subjects[sub].books[b].quantity--;
        printf("\n>>> Book '%s' by '%s' issued successfully! Remaining quantity: %d\n",
            lib.streams[s].subjects[sub].books[b].name,
            lib.streams[s].subjects[sub].books[b].author,
            lib.streams[s].subjects[sub].books[b].quantity);
    } else {
        printf("\n>>> Sorry, book '%s' by '%s' is not available at the moment.\n",
            lib.streams[s].subjects[sub].books[b].name,
            lib.streams[s].subjects[sub].books[b].author);
    }
    waitForEnter();
}

void returnBook() {
    system("clear");
    printf("\n=============================\n");
    printf("         RETURN BOOK         \n");
    printf("=============================\n");
    // Reuse filtering for selection but display in tabular format for clarity
    printf("\nTo return a book, first select the stream and subject.\n");
    filterBooksByStreamAndSubject(); 

    printf("\nEnter stream number to return book to: ");
    int s; scanf("%d", &s);
    clearInput();
    if (s < 1 || s > lib.streamCount) {
        printf(">>> Invalid stream number. Please try again.\n");
        waitForEnter();
        return;
    }
    s--;

    printf("Enter subject number: ");
    int sub; scanf("%d", &sub);
    clearInput();
    if (sub < 1 || sub > lib.streams[s].subjectCount) {
        printf(">>> Invalid subject number. Please try again.\n");
        waitForEnter();
        return;
    }
    sub--;

    printf("Enter book number: ");
    int b; scanf("%d", &b);
    clearInput();
    if (b < 1 || b > lib.streams[s].subjects[sub].bookCount) {
        printf(">>> Invalid book number. Please try again.\n");
        waitForEnter();
        return;
    }
    b--;

    lib.streams[s].subjects[sub].books[b].quantity++;
    printf("\n>>> Book '%s' by '%s' returned successfully! New quantity: %d\n",
        lib.streams[s].subjects[sub].books[b].name,
        lib.streams[s].subjects[sub].books[b].author,
        lib.streams[s].subjects[sub].books[b].quantity);
    waitForEnter();
}

// --- Admin menu ---
void adminMenu() {
    while (1) {
        system("clear");
        printf("\n=====================================\n");
        printf("         ADMINISTRATOR MENU          \n");
        printf("=====================================\n\n");
        printf("1. View All Books\n");
        printf("2. Search Book\n");
        printf("3. Filter Books by Stream & Subject\n");
        printf("4. Logout\n");
        printSeparator('-', 30);
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        printf("\n");
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: filterBooksByStreamAndSubject(); break;
            case 4:
                printf("Logging out from Admin account...\n");
                waitForEnter();
                return;
            default:
                printf(">>> Invalid choice. Please enter a number between 1 and 4.\n");
                waitForEnter();
        }
    }
}

// --- Student menu ---
void studentMenu() {
    while (1) {
        system("clear");
        printf("\n=====================================\n");
        printf("           STUDENT MENU              \n");
        printf("=====================================\n\n");
        printf("1. View All Books\n");
        printf("2. Search Book\n");
        printf("3. Filter Books by Stream & Subject\n");
        printf("4. Issue Book\n");
        printf("5. Return Book\n");
        printf("6. Logout\n");
        printSeparator('-', 30);
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        printf("\n");
        switch (choice) {
            case 1: displayBooks(); break;
            case 2: searchBook(); break;
            case 3: filterBooksByStreamAndSubject(); break;
            case 4: issueBook(); break;
            case 5: returnBook(); break;
            case 6:
                printf("Logging out from Student account...\n");
                waitForEnter();
                return;
            default:
                printf(">>> Invalid choice. Please enter a number between 1 and 6.\n");
                waitForEnter();
        }
    }
}

// --- Login system ---
void loginSystem() {
    while (1) {
        system("clear");
        printf("\n************************************************\n");
        printf("* WELCOME TO THE LIBRARY MANAGEMENT SYSTEM        *\n");
        printf("************************************************\n\n");
        printf("1. Admin Login\n");
        printf("2. Student Login\n");
        printf("3. Student Signup\n");
        printf("4. Exit\n");
        printSeparator('-', 30);
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        clearInput();
        printf("\n");

        if(choice==1){
            char adminUser[50], adminPass[PASSWORD_LENGTH];
            system("clear");
            printf("\n=============================\n");
            printf("        ADMIN LOGIN          \n");
            printf("=============================\n\n");
            printf("Admin username: ");
            fgets(adminUser, sizeof(adminUser), stdin);
            adminUser[strcspn(adminUser, "\n")] = 0;

            printf("Admin password: ");
            getPassword(adminPass, PASSWORD_LENGTH);

            if (strcmp(adminUser, "admin") == 0 && strcmp(adminPass, "Admin@123") == 0) {
                printf("\n>>> Admin login successful!\n");
                waitForEnter();
                adminMenu();
            } else {
                printf("\n>>> ERROR: Invalid admin credentials. Access Denied.\n");
                waitForEnter();
            }
        }
        else if(choice==2){
            if(studentLogin())
                studentMenu();
        }
        else if(choice==3){
            signup();
        }
        else if(choice==4){
            break;
        }
        else{
            printf(">>> Invalid choice. Please enter a number between 1 and 4.\n");
            waitForEnter();
        }
    }
}