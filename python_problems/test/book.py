class Book:
    def __init__(self, title, author, page_length):
        self.title = title
        self.author = author
        self.page_length = page_length

    def __repr__(self):
        return f"'{self.title}' by {self.author}, {self.page_length} pages"
