from book import Book
from library import Library

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
    library.add_book(Book("1984", "George Orwell", 328))

    # Search for a book
    search_result = library.search_book("To Kill a Mockingbird")
    if search_result:
        print(f"Book found: {search_result}")
    else:
        print("Book not found.")
    
    filtered_books = list(filter(lambda book: book.page_length > 200, library))
    for book in filtered_books:
        print(book)
