class Book:
    def __init__(self, title, author, page_length):
        self.title = title
        self.author = author
        self.page_length = page_length

    def __repr__(self):
        return f"'{self.title}' by {self.author}, {self.page_length} pages"

class Library:
    def __init__(self):
        self.books = {}

    def add_book(self, book):
        """Add a book to the library."""
        self.books[book.title] = book

    def remove_book(self, title):
        """Remove a book from the library."""
        if title in self.books:
            del self.books[title]
        else:
            raise ValueError(f"Book '{title}' not found in the library.")

    def search_book(self, title):
        """Search for a book by its title. Case sensitive."""
        if title in self.books:
            return self.books[title]
        return None

    def __len__(self):
        """Return the number of books in the library."""
        return len(self.books)

    def __iter__(self):
        """Return an iterator over the books in the library."""
        return iter(self.books.values())

if __name__ == '__main__':
    # Example usage
    library = Library()
    library.add_book(Book("1984", "George Orwell", 328))
    library.add_book(Book("To Kill a Mockingbird", "Harper Lee", 281))
    library.add_book(Book("The Great Gatsby", "F. Scott Fitzgerald", 180))

    print(f"Number of books in the library: {len(library)}")

    for book in library:
        print(book)

    library.remove_book("1984")
    print(f"Number of books in the library after removal: {len(library)}")

    # Search for a book
    search_result = library.search_book("To Kill a Mockingbird")
    if search_result:
        print(f"Book found: {search_result}")
    else:
        print("Book not found.")