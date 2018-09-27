class event:
    def __init__(self, day, time, name, participants)
        self.day = day
        self.start = start
        self.end = end
        self.name = name
        self.participants = participants

    def __lt__(self, other):
        return ((self.day < other.day) or
               (self.day == other.day and self.start < other.start) or
               (self.day == other.day and self.start == other.start and self.name < other.name))

    def __repr__(self):
        result = self.name + " " + self.day + " " + self.start + " " + self.end
        for p in self.participants:
            result += p + ","
        return result[:-1]