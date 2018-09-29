class event:
    def __init__(self, name, date, start, end, participants):
        self.date = date
        self.start = start
        self.end = end
        self.name = name
        self.participants = participants

    def __lt__(self, other):
        return ((self.date < other.date) or
               (self.date == other.date and self.start < other.start) or
               (self.date == other.date and self.start == other.start and self.name < other.name))

    def __repr__(self):
        result = self.name + " " + self.date + " " + self.start + " " + self.end + " "
        result += ",".join(self.participants)
        return result