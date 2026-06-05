#include "HelloWorld.hpp"
#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace
{

// -----------------------------------------------------------------------
// Grade_Book — maps a student id to a (name, score) pair. Add-only plus
// fail-safe removal; a student is never mutated once added, and the map
// key enforces one student per id. Storage is a std::map whose value is a
// std::tuple — a positional pair (ALL-4) — while the outward Student is a
// small named struct, since a *named* record reads better as a struct than
// a tuple and still gets structured bindings for free.
// -----------------------------------------------------------------------
class Grade_Book
{
public:
	// Public API type: a small aggregate of (id, name, score). A *named*
	// record reads better as a struct than a std::tuple, and an aggregate
	// still gets structured bindings for free — no inheritance, no tuple-
	// protocol boilerplate. (std::tuple stays the right tool for the
	// positional Roster value below.)
	struct Student
	{
		int32_t		m_id	= 0;
		std::string	m_name	= {};
		double		m_score	= 0.0;
	};

	// F.18: take name by value and move it to avoid an unnecessary copy.
	// Reports its outcome through the return value (ALL-20): a duplicate id is
	// rejected, never overwritten, so a re-add cannot mutate an existing student.
	bool	add_student(int32_t in_id, std::string in_name, double in_score)
	{
		const auto [position, is_inserted] = m_students.try_emplace(in_id, std::move(in_name), in_score);
		if (is_inserted && m_on_student_added)
		{
			m_on_student_added(from_entry(*position));
		}
		return is_inserted;
	}

	// Fail-safe teardown (ALL-21): removing an absent id is a successful no-op,
	// so there is nothing to report and nothing for the caller to branch on.
	void	remove_student(int32_t in_id)
	{
		m_students.erase(in_id);
	}

	// Accepts any callable matching the signature — stored for later use
	void	set_on_student_added(std::function<void(const Student&)> in_callback)
	{
		m_on_student_added = std::move(in_callback);
	}

	[[nodiscard]] std::optional<Student>	find_student(int32_t in_id) const
	{
		const auto found = m_students.find(in_id);
		if (found == m_students.end())
		{
			return std::nullopt;
		}

		return from_entry(*found);
	}

	[[nodiscard]] double	average_score() const
	{
		if (m_students.empty())
		{
			return 0.0;
		}

		// Project each entry to its score and fold (ALL-4/ALL-5): no loop, no bare get<>.
		auto scores = (m_students | std::views::values | std::views::elements<1>);
		const double total = std::accumulate(scores.begin(), scores.end(), 0.0);

		return total / static_cast<double>(m_students.size());
	}

	void	for_each(std::function<void(const Student&)> in_visitor) const
	{
		for (const auto& entry : m_students)
		{
			in_visitor(from_entry(entry));
		}
	}

	[[nodiscard]] std::vector<Student>	filter(std::function<bool(const Student&)> in_predicate) const
	{
		std::vector<Student> result;
		for (const auto& entry : m_students)
		{
			Student student = from_entry(entry);
			if (in_predicate(student))
			{
				result.push_back(std::move(student));
			}
		}
		return result;
	}

	[[nodiscard]] std::vector<Student>	sorted_by(std::function<bool(const Student&, const Student&)> in_compare) const
	{
		std::vector<Student> result;
		result.reserve(m_students.size());
		for (const auto& entry : m_students)
		{
			result.push_back(from_entry(entry));
		}
		std::sort(result.begin(), result.end(), in_compare);
		return result;
	}

private:
	// How a student is stored: id -> (name, score). The id is the key, so it
	// is not repeated in the value. This type never escapes the class.
	using Roster = std::map<int32_t, std::tuple<std::string, double>>;

	// The single bridge from a stored entry to the outward Student.
	[[nodiscard]] static Student	from_entry(const Roster::value_type& in_entry)
	{
		const auto& [name, score] = in_entry.second;
		return Student{.m_id = in_entry.first, .m_name = name, .m_score = score};
	}

	Roster								m_students;
	std::function<void(const Student&)>	m_on_student_added;
};

// Prints a vector of students using its named fields.
void	print_collection(const std::vector<Grade_Book::Student>& in_students)
{
	for (const auto& student : in_students)
	{
		std::cout << std::format("  [{:3d}]  {:<20s}  {:5.1f}\n", student.m_id, student.m_name, student.m_score);
	}
}

} // namespace

// -----------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------
void	run_demo_cpp(void)
{
	std::cout << "\n=== C++ Demo ===\n\n";

	Grade_Book book;

	// Stored closure: fires each time a student is added
	book.set_on_student_added(	[](const Grade_Book::Student& in_student)
								{
									std::cout << std::format("  + Added: {}\n", in_student.m_name);
								});

	book.add_student(1, "Alice",	92.5);
	book.add_student(2, "Bob",		78.0);
	book.add_student(3, "Carol",	85.5);
	book.add_student(4, "Dave",		61.0);
	book.add_student(5, "Eve",		97.0);

	// add_student reports its outcome through the return value (ALL-20): a
	// duplicate id is rejected, not overwritten.
	const bool is_added = book.add_student(3, "Mallory", 10.0);
	std::cout << std::format("\nRe-adding id 3: {}\n", is_added ? "inserted" : "rejected (id already present)");

	std::cout << "\nAll students:\n";
	book.for_each(	[](const Grade_Book::Student& in_student)
					{
						std::cout << std::format("  [{:3d}]  {:<20s}  {:5.1f}\n", in_student.m_id, in_student.m_name, in_student.m_score);
					});

	std::cout << std::format("\nAverage: {:.1f}\n", book.average_score());

	// Lambda capturing by value — safe because the lambda does not outlive threshold
	double threshold = 80.0;
	auto high_achievers = book.filter([threshold](const Grade_Book::Student& in_student) { return in_student.m_score >= threshold; });
	std::cout << std::format("\nStudents scoring >= {:.0f}:\n", threshold);
	print_collection(high_achievers);

	// Comparator lambda passed to sorted_by
	auto by_score_desc = book.sorted_by([](const Grade_Book::Student& in_lhs, const Grade_Book::Student& in_rhs) { return in_lhs.m_score > in_rhs.m_score; });
	std::cout << "\nRanked by score:\n";
	print_collection(by_score_desc);

	// Mutable lambda: captures a counter by value and mutates it across calls.
	// Without 'mutable', modifying a captured-by-value variable is a compile error.
	int32_t rank = 0;
	auto print_ranked =	[rank](const Grade_Book::Student& in_student) mutable
						{
							std::cout << std::format("  #{}: {}\n", ++rank, in_student.m_name);
						};
	std::cout << "\nRanks (via mutable lambda):\n";
	for (const auto& student : by_score_desc)
	{
		print_ranked(student);
	}

	// IILE: the result must be const and the computation requires iterating
	// over all students — a named helper or mutable temporary are the alternatives.
	const double top_score =	[&book]()
								{
									double max_score = 0.0;
									book.for_each(	[&max_score](const Grade_Book::Student& in_student)
													{
														if (in_student.m_score > max_score)
														{
															max_score = in_student.m_score;
														}
													});
									return max_score;
								}();
	std::cout << std::format("\nTop score: {:.1f}\n", top_score);

	// std::optional: query result that may be absent
	if (auto found = book.find_student(3))
	{
		std::cout << std::format("\nFound student 3: {}\n", found->m_name);
	}

	if (!book.find_student(99))
	{
		std::cout << "Student 99: not found\n";
	}

	// Fail-safe teardown (ALL-21): remove returns nothing, and removing an
	// absent id is a harmless no-op.
	book.remove_student(2);
	book.remove_student(99);
	std::cout << "\nAfter removing id 2 (and a no-op remove of id 99):\n";
	print_collection(book.sorted_by([](const Grade_Book::Student& in_lhs, const Grade_Book::Student& in_rhs) { return in_lhs.m_id < in_rhs.m_id; }));
}

// -----------------------------------------------------------------------
// Hello
// -----------------------------------------------------------------------
void	hello_cpp(void)
{
	std::cout << "Hello from C++" << std::endl;
}
