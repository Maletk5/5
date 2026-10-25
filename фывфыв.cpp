#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <limits>
#define NOMINMAX          
#include <Windows.h>
#pragma execution_character_set("utf-8")
using namespace std;
class Reader;
class Book {
private:
    int id;
    string title;
    string author;
    int year;
    weak_ptr<Reader> currentHolder;
public:
    Book(int id, string title, string author, int year)
        : id(id), title(move(title)), author(move(author)), year(year) {
    }
    int getId() const { return id; }
    const string& getTitle() const { return title; }
    const string& getAuthor() const { return author; }
    int getYear() const { return year; }
    void setTitle(const string& t) { title = t; }
    void setAuthor(const string& a) { author = a; }
    void setYear(int y) { year = y; }
    bool isAvailable() const { return currentHolder.expired(); }
    shared_ptr<Reader> getHolder() const { return currentHolder.lock(); }
    void setHolder(const shared_ptr<Reader>& r) { currentHolder = r; }
    void clearHolder() { currentHolder.reset(); }
    void print() const;
};
class Reader {
private:
    int id;
    string name;
    string phone;
    vector<shared_ptr<Book>> borrowedBooks;
public:
    Reader(int id, string name, string phone)
        : id(id), name(move(name)), phone(move(phone)) {
    }
    int getId() const { return id; }
    const string& getName() const { return name; }
    const string& getPhone() const { return phone; }
    void setName(const string& n) { name = n; }
    void setPhone(const string& p) { phone = p; }
    const vector<shared_ptr<Book>>& getBorrowedBooks() const { return borrowedBooks; }
    void addBook(const shared_ptr<Book>& b) { borrowedBooks.push_back(b); }
    bool removeBook(int bookId) {
        auto it = find_if(borrowedBooks.begin(), borrowedBooks.end(),
            [bookId](const shared_ptr<Book>& b) { return b->getId() == bookId; });
        if (it != borrowedBooks.end()) {
            borrowedBooks.erase(it);
            return true;
        }
        return false;
    }
    void print() const;
};
void Book::print() const {
    cout << "  ID: " << id
        << " \"" << title << "\""
        << "  Автор: " << author
        << "  Год: " << year
        << "  Статус: " << (isAvailable() ? "доступна" : "выдана");
    if (auto h = getHolder())
        cout << " (читатель: " << h->getName() << ")";
    cout << "\n";
}
void Reader::print() const {
    cout << "  ID: " << id
        << "  " << name
        << "  Тел: " << phone
        << "  Книг на руках: " << borrowedBooks.size() << "\n";
}
class Library {
private:
    vector<shared_ptr<Book>>   books;
    vector<shared_ptr<Reader>> readers;
    int nextBookId = 1;
    int nextReaderId = 1;
    int readInt(const string& prompt) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ошибка ввода. Попробуйте снова.\n";
        }
    }
    string readString(const string& prompt) {
        string s;
        cout << prompt;
        getline(cin, s);
        return s;
    }
    shared_ptr<Book> findBook(int id) {
        for (auto& b : books) if (b->getId() == id) return b;
        return nullptr;
    }
    shared_ptr<Reader> findReader(int id) {
        for (auto& r : readers) if (r->getId() == id) return r;
        return nullptr;
    }
public:
    Library() { initializeData(); }
    void initializeData() {
        books.push_back(make_shared<Book>(nextBookId++, "Война и мир", "Л.Н. Толстой", 1869));
        books.push_back(make_shared<Book>(nextBookId++, "Преступление и наказание", "Ф.М. Достоевский", 1866));
        books.push_back(make_shared<Book>(nextBookId++, "Мастер и Маргарита", "М.А. Булгаков", 1967));
        books.push_back(make_shared<Book>(nextBookId++, "Евгений Онегин", "А.С. Пушкин", 1833));
        books.push_back(make_shared<Book>(nextBookId++, "Отцы и дети", "И.С. Тургенев", 1862));

        readers.push_back(make_shared<Reader>(nextReaderId++, "Иван Иванов", "+7-900-111-22-33"));
        readers.push_back(make_shared<Reader>(nextReaderId++, "Мария Петрова", "+7-900-444-55-66"));
        auto b = books.front();
        auto r = readers.front();
        b->setHolder(r);
        r->addBook(b);
    }
    void addBook() {
        string title = readString("Название: ");
        string author = readString("Автор: ");
        int year = readInt("Год издания: ");
        int id = nextBookId++;
        books.push_back(make_shared<Book>(id, title, author, year));
        cout << "Книга добавлена (ID " << id << ")" "\n";
    }

    void listBooks() {
        if (books.empty()) { cout << "Список книг пуст.\n"; return; }
        cout << "Книги:\n";
        for (const auto& b : books) b->print();
    }
    void updateBook() {
        int id = readInt("ID книги для редактирования: ");
        auto b = findBook(id);
        if (!b) { cout << "Книга не найдена.\n"; return; }
        cout << "Текущие данные:\n"; b->print();
        string title = readString("Новое название (Enter — оставить): ");
        if (!title.empty()) b->setTitle(title);
        string author = readString("Новый автор (Enter — оставить): ");
        if (!author.empty()) b->setAuthor(author);
        string yearStr = readString("Новый год (Enter — оставить): ");
        if (!yearStr.empty()) {
            try { b->setYear(stoi(yearStr)); }
            catch (...) { cout << "Неверный год, оставлено старое значение.\n"; }
        }
        cout << "Данные обновлены.\n";
    }
    void deleteBook() {
        int id = readInt("ID книги для удаления: ");
        auto b = findBook(id);
        if (!b) { cout << "Книга не найдена" "\n"; return; }
        if (!b->isAvailable()) {
            cout << "Нельзя удалить выданную книгу. Сначала верните её" "\n";
            return;
        }
        books.erase(remove(books.begin(), books.end(), b), books.end());
        cout << "Книга удалена" "\n";
    }
    void addReader() {
        string name = readString("Имя читателя: ");
        string phone = readString("Телефон: ");
        int id = nextReaderId++;
        readers.push_back(make_shared<Reader>(id, name, phone));
        cout << "Читатель добавлен (ID " << id << ").\n";
    }
    void listReaders() {
        if (readers.empty()) { cout << "Список читателей пуст" "\n"; return; }
        cout << "Читатели:" "\n";
        for (const auto& r : readers) r->print();
    }

    void updateReader() {
        int id = readInt("ID читателя: ");
        auto r = findReader(id);
        if (!r) { cout << "Читатель не найден.\n"; return; }
        cout << "Текущие данные:\n"; r->print();
        string name = readString("Новое имя (Enter — оставить): ");
        if (!name.empty()) r->setName(name);
        string phone = readString("Новый телефон (Enter — оставить): ");
        if (!phone.empty()) r->setPhone(phone);
        cout << "Данные обновлены" "\n";
    }

    void deleteReader() {
        int id = readInt("ID читателя для удаления: ");
        auto r = findReader(id);
        if (!r) { cout << "Читатель не найден" "\n"; return; }
        readers.erase(remove(readers.begin(), readers.end(), r), readers.end());
        cout << "Читатель удалён. Все его книги возвращены в фонд" "\n";
    }
    void lendBook() {
        int bookId = readInt("ID книги: ");
        auto b = findBook(bookId);
        if (!b) { cout << "Книга не найдена" "\n"; return; }
        if (!b->isAvailable()) {
            cout << "Книга уже выдана";
            if (auto h = b->getHolder()) cout << " читателю " << h->getName();
            cout << ".\n";
            return;
        }
        int readerId = readInt("ID читателя: ");
        auto r = findReader(readerId);
        if (!r) { cout << "Читатель не найден" "\n"; return; }
        b->setHolder(r);
        r->addBook(b);
        cout << "Книга " << b->getTitle() << " выдана читателю "
            << r->getName() << "\n";
    }
    void returnBook() {
        int bookId = readInt("ID возвращаемой книги: ");
        auto b = findBook(bookId);
        if (!b) { cout << "Книга не найдена" "\n"; return; }
        if (b->isAvailable()) { cout << "Книга не выдана" "\n"; return; }
        if (auto h = b->getHolder()) h->removeBook(bookId);
        b->clearHolder();
        cout << "Книга " << b->getTitle() << " возвращена в фонд" "\n";
    }
    void showReaderBooks() {
        int readerId = readInt("ID читателя: ");
        auto r = findReader(readerId);
        if (!r) { cout << "Читатель не найден" "\n"; return; }
        const auto& bb = r->getBorrowedBooks();
        if (bb.empty()) {
            cout << "У читателя " << r->getName() << " нет книг на руках" "\n";
            return;
        }
        cout << "Книги читателя " << r->getName() << ":\n";
        for (const auto& b : bb) b->print();
    }

    void showStatistics() {
        int total = static_cast<int>(books.size());
        int available = 0, borrowed = 0;
        for (const auto& b : books) (b->isAvailable() ? available : borrowed)++;

        int totalReaders = static_cast<int>(readers.size());
        int active = 0;
        for (const auto& r : readers) if (!r->getBorrowedBooks().empty()) active++;

        cout << "статистика библиотеки" "\n"
            << "Всего книг:        " << total << "\n"
            << "  доступно:        " << available << "\n"
            << "  выдано:          " << borrowed << "\n"
            << "Всего читателей:   " << totalReaders << "\n"
            << "  с книгами:       " << active << "\n"
            << "  без книг:        " << (totalReaders - active) << "\n";
    }
    void booksMenu() {
        while (true) {
            cout << "управление книгами" "\n"
                << "1. Добавить книгу" "\n"
                << "2. Список книг" "\n"
                << "3. Редактировать книгу" "\n"
                << "4. Удалить книгу" "\n"
                << "0. Назад" "\n";
            int c = readInt("Выбор: ");
            switch (c) {
            case 1: addBook(); break;
            case 2: listBooks(); break;
            case 3: updateBook(); break;
            case 4: deleteBook(); break;
            case 0: return;
            default: cout << "Неверный выбор" "\n";
            }
        }
    }

    void readersMenu() {
        while (true) {
            cout << "управление ситателями" "\n"
                << "1. Добавить читателя" "\n"
                << "2. Список читателей" "\n"
                << "3. Редактировать читателя" "\n"
                << "4. Удалить читателя" "\n"
                << "0. Назад" "\n";
            int c = readInt("Выбор: ");
            switch (c) {
            case 1: addReader(); break;
            case 2: listReaders(); break;
            case 3: updateReader(); break;
            case 4: deleteReader(); break;
            case 0: return;
            default: cout << "Неверный выбор" "\n";
            }
        }
    }

    void run() {
        while (true) {
            cout << "библиотека" "\n"
                << "1. Управление книгами" "\n"
                << "2. Управление читателями" "\n"
                << "3. Выдать книгу читателю" "\n"
                << "4. Вернуть книгу" "\n"
                << "5. Показать книги читателя" "\n"
                << "6. Статистика библиотеки" "\n"
                << "0. Выход" "\n";
            int c = readInt("Выбор: ");
            switch (c) {
            case 1: booksMenu(); break;
            case 2: readersMenu(); break;
            case 3: lendBook(); break;
            case 4: returnBook(); break;
            case 5: showReaderBooks(); break;
            case 6: showStatistics(); break;
            case 0: cout << "GGWP" "\n"; return;
            default: cout << "Неверный выбор" "\n";
            }
        }
    }
};

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    Library library;
    library.run();
    return 0;
}