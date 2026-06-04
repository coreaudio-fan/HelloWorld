#import <Foundation/Foundation.h>
#include "HelloWorld_objc.h"
#include <stdio.h>

NS_ASSUME_NONNULL_BEGIN

// Block typedefs give complex block signatures a readable name.
// Without a typedef, block parameters in method signatures quickly become unreadable.
// Note: block properties must be declared 'copy' (not 'strong') so the block
// is copied from the stack to the heap when assigned.
typedef void	(^HWStudentBlock)(NSString* name, float score);
typedef BOOL	(^HWStudentPredicate)(NSString* name, float score);

// Private student model — used only inside this file
@interface HWStudent : NSObject
@property (nonatomic, assign)	int32_t		studentId;
@property (nonatomic, copy)		NSString*	name;
@property (nonatomic, assign)	float		score;
- (instancetype)initWithId:(int32_t)studentId name:(NSString*)name score:(float)score;
@end

@implementation HWStudent

- (instancetype)initWithId:(int32_t)studentId name:(NSString*)name score:(float)score {
	self = [super init];
	if (self) {
		_studentId	= studentId;
		_name		= [name copy];
		_score		= score;
	}
	return self;
}

@end

// Class prefix 'HW' follows the Objective-C convention of prefixing type names
// to avoid collisions in the global namespace.
@interface HWGradeBook : NSObject
@property (nonatomic, copy, nullable)	HWStudentBlock				onStudentAdded;
@property (nonatomic, strong)			NSMutableArray<HWStudent*>*	students;

- (void)addStudentWithId:(int32_t)studentId
                    name:(NSString*)name
                   score:(float)score;

- (nullable NSString*)nameOfStudentWithId:(int32_t)studentId;
- (float)averageScore;
- (void)enumerateStudentsUsingBlock:(HWStudentBlock)block;
- (NSArray<NSString*>*)namesOfStudentsMatchingPredicate:(HWStudentPredicate)predicate;

+ (instancetype)gradeBookWithStudents:(NSArray<NSDictionary<NSString*, id>*>*)students;
@end

@implementation HWGradeBook

- (instancetype)init {
	self = [super init];
	if (self) {
		_students = [NSMutableArray array];
	}
	return self;
}

+ (instancetype)gradeBookWithStudents:(NSArray<NSDictionary<NSString*, id>*>*)dicts {
	HWGradeBook* book = [[HWGradeBook alloc] init];
	for (NSDictionary* dict in dicts) {
		[book addStudentWithId:[dict[@"id"] intValue]
		                 name:dict[@"name"]
		                score:[dict[@"score"] floatValue]];
	}
	return book;
}

- (void)addStudentWithId:(int32_t)studentId name:(NSString*)name score:(float)score {
	HWStudent* student = [[HWStudent alloc] initWithId:studentId name:name score:score];
	[self.students addObject:student];
	if (self.onStudentAdded) {
		self.onStudentAdded(name, score);
	}
}

- (nullable NSString*)nameOfStudentWithId:(int32_t)studentId {
	for (HWStudent* student in self.students) {
		if (student.studentId == studentId)
			return student.name;
	}
	return nil;
}

- (float)averageScore {
	if (self.students.count == 0)
		return 0.0f;
	float total = 0.0f;
	for (HWStudent* student in self.students)
		total += student.score;
	return total / (float)self.students.count;
}

- (void)enumerateStudentsUsingBlock:(HWStudentBlock)block {
	for (HWStudent* student in self.students)
		block(student.name, student.score);
}

- (NSArray<NSString*>*)namesOfStudentsMatchingPredicate:(HWStudentPredicate)predicate {
	NSMutableArray<NSString*>* result = [NSMutableArray array];
	for (HWStudent* student in self.students) {
		if (predicate(student.name, student.score))
			[result addObject:student.name];
	}
	return [result copy];
}

@end

NS_ASSUME_NONNULL_END

// -----------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------
void	run_demo_objc(void) {
	printf("\n=== Objective-C Demo ===\n\n");

	HWGradeBook* book = [[HWGradeBook alloc] init];

	// Stored block: captures no external state here, but illustrates the pattern.
	// The property is declared 'copy' so the block is heap-allocated.
	book.onStudentAdded = ^(NSString* name, float score) {
		printf("  + Added: %s (%.1f)\n", name.UTF8String, score);
	};

	[book addStudentWithId:1 name:@"Alice"	score:92.5f];
	[book addStudentWithId:2 name:@"Bob"	score:78.0f];
	[book addStudentWithId:3 name:@"Carol"	score:85.5f];
	[book addStudentWithId:4 name:@"Dave"	score:61.0f];
	[book addStudentWithId:5 name:@"Eve"	score:97.0f];

	printf("\nAll students:\n");
	[book enumerateStudentsUsingBlock:^(NSString* name, float score) {
		printf("  %-20s  %.1f\n", name.UTF8String, score);
	}];

	printf("\nAverage: %.1f\n", book.averageScore);

	// Filter with a predicate block — inline block as trailing argument
	NSArray<NSString*>* high_achievers = [book namesOfStudentsMatchingPredicate:^BOOL(NSString* name, float score) {
		return name.length > 0 && score >= 80.0f;
	}];
	printf("\nStudents scoring >= 80:\n");
	for (NSString* name in high_achievers)
		printf("  %s\n", name.UTF8String);

	// __weak/__strong dance: prevents a retain cycle when a block captures an
	// object that owns the block. Without __weak, each would keep the other alive.
	__weak HWGradeBook* weak_book = book;
	HWStudentBlock snapshot_block = ^(NSString* name, float score) {
		__strong HWGradeBook* strong_book = weak_book;
		if (strong_book != nil) {
			printf("  [snapshot] %-20s %.1f  (avg: %.1f)\n", name.UTF8String, score, strong_book.averageScore);
		}
	};
	printf("\nEnumerating with weak/strong capture pattern:\n");
	[book enumerateStudentsUsingBlock:snapshot_block];

	// Dot notation vs message syntax — both compile; style guides disagree.
	// Apple's own code mixes both; many teams pick one and enforce it.
	float avg_dot		= book.averageScore;        // dot notation (property access style)
	float avg_message	= [book averageScore];       // message syntax (traditional Objective-C)
	printf("\navg (dot): %.1f   avg (message): %.1f\n", avg_dot, avg_message);

	// Factory class method
	HWGradeBook* book2 = [HWGradeBook gradeBookWithStudents:@[
		@{@"id": @10, @"name": @"Frank", @"score": @88.0},
		@{@"id": @11, @"name": @"Grace", @"score": @74.5},
	]];
	printf("\nBook2 average: %.1f\n", book2.averageScore);
}

// -----------------------------------------------------------------------
// Hello
// -----------------------------------------------------------------------
void	hello_objc(void) {
	NSString* msg = @"Hello from Objective-C";
	fprintf(stdout, "%s\n", msg.UTF8String);
}
