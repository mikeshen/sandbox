from book import Book

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
        return self.books.get(title, None)

    def __len__(self):
        """Return the number of books in the library."""
        return len(self.books)

    def __iter__(self):
        """Return an iterator over the books in the library."""
        return iter(self.books.values())