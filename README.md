# Library Management System in C

A complete terminal-based Library Management System built in C.  
It supports Admin and Student roles, secure logins, book issuing/returning with stock tracking, fine calculation, and categorized book search.


##  Features

 **Admin Module**
- View all books in stock
- Track issued/returned books
- Add new books
- See filtered books by Stream/Subject
- Export reports

 **Student Module**
- Secure signup/login with strong password rules
- Password masking without `conio.h`
- Browse/filter books (Stream → Subject → Titles)
- Issue/Return books (with quantity update)
- View issued books history
- Fine calculation for late returns

 **Role-Based Menus**
- Separate, intuitive text UI for Admin and Student
- Context-sensitive messages and prompts

 **Streams & Subjects**
- Supports categorization like BCA, MCA, BTech, etc.
- Subjects within streams for easy filtering

 **File Handling**
- Persistent storage for:
  - Book inventory
  - Issued/returned books records
  - Student login credentials
  - Transaction history

---

##  Project Structure
```
LibraryManagementSystem/
├── main.c             → Entry point with menus
├── admin.c            → Admin functions
├── student.c          → Student functions
├── books.c            → Book data handling
├── files.c            → File I/O utilities
└── README.md          → Project documentation
```

# Password Policy

- At least 1 uppercase letter
- At least 1 lowercase letter
- At least 1 digit
- At least 1 special character
- Minimum length enforced
- Masked input (even without using `conio.h`)


# How to Compile and Run

# On Linux / macOS
```bash
gcc main.c -o library
./library
```


Technologies Used
	•	C Programming Language
	•	File I/O (text/binary as implemented)
	•	Structured programming with modular functions

My Role : 
```
-Designed the full system architecture
-Implemented secure login with password validation
-Developed Admin and Student workflows
-Managed inventory updates on issue/return
-Built file handling for persistence
-Ensured user-friendly CLI experience
