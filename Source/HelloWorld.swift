import Foundation

// -----------------------------------------------------------------------
// Grade — maps score ranges to letter grades.
// CaseIterable allows exhaustive iteration; the compiler catches any
// new case added to the enum that isn't handled in a switch.
// -----------------------------------------------------------------------
enum Grade: String, CaseIterable {
	case a = "A"
	case b = "B"
	case c = "C"
	case d = "D"
	case f = "F"

	static func from(score: Double) -> Grade {
		switch score {
		case 90...100: return .a
		case 80..<90:  return .b
		case 70..<80:  return .c
		case 60..<70:  return .d
		default:       return .f
		}
	}
}

// -----------------------------------------------------------------------
// StudentError — typed errors with associated values.
// Associated value labels make call sites self-documenting.
// -----------------------------------------------------------------------
enum StudentError: Error {
	case notFound(id: Int32)
	case duplicateId(Int32)
	case invalidScore(Double)
}

// Protocol conformance in a separate extension — a common Swift pattern
// that keeps the type definition uncluttered.
extension StudentError: CustomStringConvertible {
	var description: String {
		switch self {
		case .notFound(let id):         return "Student \(id) not found"
		case .duplicateId(let id):      return "Duplicate id: \(id)"
		case .invalidScore(let score):  return "Invalid score: \(score)"
		}
	}
}

// -----------------------------------------------------------------------
// SortOrder — replaces a Bool parameter with a meaningful name.
// 'sorted(ascending: true)' is far less readable than 'sorted(by: .ascending)'.
// -----------------------------------------------------------------------
enum SortOrder {
	case ascending
	case descending
}

// -----------------------------------------------------------------------
// Student — value type (struct) with a computed property.
// -----------------------------------------------------------------------
struct Student {
	let id: Int32
	let name: String
	let score: Double

	var grade: Grade { Grade.from(score: score) }
}

extension Student: CustomStringConvertible {
	var description: String { "[\(id)] \(name)  \(String(format: "%.1f", score))  (\(grade.rawValue))" }
}

// -----------------------------------------------------------------------
// GradeBook — reference type (class) so mutations are visible to all holders.
// Choosing class vs struct here is one of the most common Swift debates;
// structs are preferred by default, but reference semantics are appropriate
// when identity or shared mutable state is needed.
// -----------------------------------------------------------------------
class GradeBook {

	private var students: [Student] = []

	// @escaping: the closure is stored and called after addStudent returns,
	// so it must be marked escaping to allow capture beyond the call site.
	var onStudentAdded: ((Student) -> Void)?

	// @discardableResult: callers that don't need the returned Student
	// won't receive a compiler warning for ignoring it.
	@discardableResult
	func addStudent(id: Int32, name: String, score: Double) throws -> Student {
		guard score >= 0 && score <= 100 else {
			throw StudentError.invalidScore(score)
		}
		guard !students.contains(where: { $0.id == id }) else {
			throw StudentError.duplicateId(id)
		}
		let student = Student(id: id, name: name, score: score)
		students.append(student)
		onStudentAdded?(student)
		return student
	}

	// Returns Result rather than Optional so the caller knows *why* lookup failed.
	func findStudent(withId id: Int32) -> Result<Student, StudentError> {
		guard let student = students.first(where: { $0.id == id }) else {
			return .failure(.notFound(id: id))
		}
		return .success(student)
	}

	func averageScore() -> Double {
		guard !students.isEmpty else { return 0.0 }
		return students.reduce(0.0) { $0 + $1.score } / Double(students.count)
	}

	func forEach(do body: (Student) -> Void) {
		students.forEach(body)
	}

	func filter(matching predicate: (Student) -> Bool) -> [Student] {
		students.filter(predicate)
	}

	func sorted(by order: SortOrder = .ascending) -> [Student] {
		switch order {
		case .ascending:  return students.sorted { $0.score < $1.score }
		case .descending: return students.sorted { $0.score > $1.score }
		}
	}
}

// -----------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------
func runDemoSwift() {
	print("\n=== Swift Demo ===\n")

	let book = GradeBook()

	// Stored escaping closure: assigned before any students are added so it
	// fires for each addStudent call below.
	book.onStudentAdded = { student in
		print("  + Added: \(student.name)")
	}

	// [weak book] prevents a retain cycle: if book owned this closure and the
	// closure captured book strongly, neither could ever be deallocated.
	// guard let turns the weak reference into a strong one for the closure body.
	let logAdded: (Student) -> Void = { [weak book] student in
		guard let book = book else { return }
		print("  [log] after \(student.name): avg = \(String(format: "%.1f", book.averageScore()))")
	}

	try? book.addStudent(id: 1, name: "Alice", score: 92.5)
	try? book.addStudent(id: 2, name: "Bob",   score: 78.0)
	try? book.addStudent(id: 3, name: "Carol", score: 85.5)
	try? book.addStudent(id: 4, name: "Dave",  score: 61.0)
	try? book.addStudent(id: 5, name: "Eve",   score: 97.0)

	// Calling the [weak book] closure explicitly to illustrate the pattern
	print("\nReplay with weak/strong capture:")
	book.forEach { logAdded($0) }

	print("\nAll students:")
	book.forEach { print("  \($0)") }

	print(String(format: "\nAverage: %.1f", book.averageScore()))

	// Trailing closure syntax — idiomatic Swift when the closure is the last argument
	let highAchievers = book.filter { $0.score >= 80 }
	print("\nStudents scoring >= 80:")
	highAchievers.forEach { print("  \($0.name)") }

	// Chained higher-order functions with shorthand $0 argument names.
	// Debate: is a single chain more readable, or should intermediate
	// results be named variables?
	let topNames = book.sorted(by: .descending)
		.filter { $0.grade == .a || $0.grade == .b }
		.map { $0.name }
	print("\nTop students (A or B): \(topNames.joined(separator: ", "))")

	// switch on Result — exhaustive, no default:
	switch book.findStudent(withId: 3) {
	case .success(let student):
		print("\nFound student 3: \(student.name) — grade \(student.grade.rawValue)")
	case .failure(let error):
		print("\nError: \(error)")
	}

	// 'if case' pattern matching — concise alternative to a full switch
	// when only one case is relevant
	if case .failure(let error) = book.findStudent(withId: 99) {
		print("Student 99: \(error)")
	}

	// Exhaustive switch over all Grade cases — no default: means the compiler
	// will warn if a new case is added to the enum without handling it here
	print("\nGrade scale:")
	for grade in Grade.allCases {
		switch grade {
		case .a: print("  A: 90-100  Excellent")
		case .b: print("  B: 80-89   Good")
		case .c: print("  C: 70-79   Average")
		case .d: print("  D: 60-69   Below average")
		case .f: print("  F:  0-59   Failing")
		}
	}

	// @autoclosure: the expression passed as 'condition' is not evaluated
	// unless the function actually inspects it — useful for assertions
	// and validation helpers where evaluation may be expensive or have side effects
	func require(_ condition: @autoclosure () -> Bool, _ message: String) {
		if !condition() { print("Requirement failed: \(message)") }
	}
	require(book.averageScore() > 0, "grade book should not be empty")
}

// -----------------------------------------------------------------------
// Hello
// -----------------------------------------------------------------------
func helloSwift() {
	print("Hello from Swift")
}
