#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <cstdint>

using u32 = uint32_t;
using f32 = float;

f32 HexToFloat(u32 Hex)
{
	return (f32&)Hex;
}

u32 FloatToHex(f32 Float)
{
	return (u32&)Float;
}

f32 HexDeltaToOffset(u32 Start, u32 End)
{
	return HexToFloat(End) - HexToFloat(Start);
}

struct Move
{
	Move(const char* InName, f32 InOffset, f32 InMinimumZ, f32 InMaximumZ, bool InBiDirectional) : Name(InName), Offset(InOffset), MinimumZ(InMinimumZ), MaximumZ(InMaximumZ), BiDirectional(InBiDirectional) {}
	const char* Name;
	f32 Offset;
	f32 MinimumZ;
	f32 MaximumZ;
	bool BiDirectional;
};

// The position two measure distances from (two backflips from crawlspace wall)
u32 BaselineStart = 0x44bf6000;

// The landing positions of different moves from baseline
int VerticalSlashEnd = 0x44bf623d;
int InterruptedSlashEnd = 0x44bf37ae;
int BigSwordSlashEnd = 0x44bf5f5c;
int WallSideSlashEnd = 0x44c1cd70;
u32 BigVerticalSlashEnd = 0x44bf5f5c;
u32 BigInterruptedVerticalSlashEnd = 0x44bf9000;
u32 BigVerticalFaceRightSlashEnd = 0x44bf5fff;
u32 BigInterruptedFaceRightSlashEnd = 0x44bfb99a;

// For ones that use the front wall
u32 FrontWallStart = 0x44a4c000;
u32 VerticalWallSlashEnd = 0x44a6cd1e;
u32 BigVerticalWallSlashEnd = 0x44ac0d70;

const Move Bonk("Bonk", 1355.5f, FLT_MIN, FLT_MAX, false);
const Move StandAtWall("StandAtFront", 1318.f, FLT_MIN, FLT_MAX, false);
const Move SlowSpawn("SlowSpawn", HexToFloat(0x44d4bb2f), FLT_MIN, FLT_MAX, false);

const Move NeutralRoll("NeutralRoll", -48.f, FLT_MIN, FLT_MAX, true);
const Move Backflip("Backflip", 106.5f, FLT_MIN, FLT_MAX, true);

const Move WallSideSlash("WallSideSlash", HexDeltaToOffset(BaselineStart, WallSideSlashEnd), 1340.f, FLT_MAX, false);
const Move VerticalSlash("VerticalSlash", HexDeltaToOffset(BaselineStart, VerticalSlashEnd), 1360.f, FLT_MAX, false);
const Move VerticalWallSlash("VerticalWallSlash", HexDeltaToOffset(FrontWallStart, VerticalWallSlashEnd), FLT_MIN, 1360.f, false);
const Move InterruptedSlash("InterruptedSlash", HexDeltaToOffset(BaselineStart, InterruptedSlashEnd), 1360.f, FLT_MAX, false);

const Move BigVerticalWallSlash("BigVerticalWallSlash", HexDeltaToOffset(FrontWallStart, BigVerticalWallSlashEnd), FLT_MIN, 1375.f, false);
const Move BigVerticalSlash("BigVerticalSlashEnd", HexDeltaToOffset(BaselineStart, BigVerticalSlashEnd), 1375.f, FLT_MAX, false);
const Move BigInterruptedVerticalSlash("BigInterruptedVerticalSlash", HexDeltaToOffset(BaselineStart, BigInterruptedVerticalSlashEnd), 1375.f, FLT_MAX, false);
const Move BigVerticalFaceRightSlash("BigVerticalFaceRightSlash", HexDeltaToOffset(BaselineStart, BigVerticalFaceRightSlashEnd), 1375.f, FLT_MAX, false);
const Move BigInterruptedVerticalFaceRightSlash("BigInterruptedVerticalFaceRightSlash", HexDeltaToOffset(BaselineStart, BigInterruptedFaceRightSlashEnd), 1375.f, FLT_MAX, false);

const Move Crouchstab("CrouchStab", 30.f, FLT_MIN, 1370.f, false);

#define BIG_SWORD 1

// Comment out moves you don't wanna use
const Move* MoveSet[] = {
	&NeutralRoll,
	&Backflip,
#if BIG_SWORD
	&BigVerticalWallSlash,
	&BigVerticalSlash,
	&BigInterruptedVerticalSlash,
	&BigVerticalFaceRightSlash,
	&BigInterruptedVerticalFaceRightSlash,
#else
	&WallSideSlash,
	&VerticalSlash,
	&VerticalWallSlash,
	&InterruptedSlash,
#endif
	&Crouchstab
};

constexpr u32 NumMoves = sizeof(MoveSet) / sizeof(Move);

struct Candidate
{
	Candidate(const Move* InWhichMove, Candidate* InParent, f32 InPosition, int InSteps, bool InForward) : WhichMove(InWhichMove), Parent(InParent), Position(InPosition), Steps(InSteps), Forward(InForward) {}
	Candidate(const Move& InStarterMove) : Candidate(&InStarterMove, nullptr, InStarterMove.Offset, 0, true) {}
	const Move* WhichMove;
	Candidate* Parent;
	f32 Position;
	u32 Steps;
	bool Forward;
};

int main(int argc, char** argv)
{
	// Build all possible outcomes
	const u32 CorrectHookshotsIntegers[] = { 0x44a5618e, 0x44a5618d, 0x44a1a79e, 0x44a1a79d, 0x449dedae, 0x449dedad, 0x449a33bd, 0x449a33bd };
	const f32 WrongStanding = 1405.750f; // Random baseline position I chose to get wrong hookshot  (one sidehop towards entrance?)
	const u32 WrongHookshotInteger = 0x44a73848; // Hookshot Z coordinate after weirdshotting from WrongStanding Z position and turning left
	const f32 WrongHookshot = HexToFloat(WrongHookshotInteger);
	std::set<f32> Destinations;
	for (u32 CorrectHookshotInteger : CorrectHookshotsIntegers)
	{
		const f32 CorrectHookshot = HexToFloat(CorrectHookshotInteger);
		const f32 Offset = CorrectHookshot - WrongHookshot;
		const f32 Destination = WrongStanding + Offset;
		Destinations.insert(Destination);
	}

#if 0
	// More from the right side with a bonk and short-drop
	const u32 MoreCorrect[] = { 0x44a4a49e, 0x44a4a49f };
	const f32 MoreWrongHookshot = HexToFloat(0x44abfdda);
	for (u32 CorrectHookshotInteger : MoreCorrect)
	{
		const f32 CorrectHookshot = HexToFloat(CorrectHookshotInteger);
		const f32 Offset = CorrectHookshot - MoreWrongHookshot;
		const f32 Destination = WrongStanding + Offset;
		Destinations.insert(Destination);
	}
#endif

#if 0
	// More from the right side with short-drop
	const u32 ShortDropCorrect[] = { 0x44a5618e, 0x44a5618d };
	const f32 ShortDropWrongHookshot = HexToFloat(0x44a7f2e1);
	for (u32 CorrectHookshotInteger : ShortDropCorrect)
	{
		const f32 CorrectHookshot = HexToFloat(CorrectHookshotInteger);
		const f32 Offset = CorrectHookshot - ShortDropWrongHookshot;
		const f32 Destination = WrongStanding + Offset;
		Destinations.insert(Destination);
	}
#endif

	const f32 StartPosition = 1318.f;
	const f32 MaximumPosition = 1800.f;
	std::map<f32, Candidate*> PositionToCandidate;
	std::vector<Candidate*> NextCandidates;
	// NextCandidates.push_back(new Candidate(SlowSpawn));
	NextCandidates.push_back(new Candidate(StandAtWall));
	NextCandidates.push_back(new Candidate(Bonk));

	unsigned long long Tested = 0;
	auto AddCandidate = [&](const f32 BasePosition, const Move* InMove, bool InForward, Candidate* InParent)
	{
		Tested++;
		if (BasePosition >= InMove->MinimumZ && BasePosition <= InMove->MaximumZ)
		{
			const f32 Offset = (InForward ? InMove->Offset : -InMove->Offset);
			const f32 NewPosition = BasePosition + Offset;
			if (NewPosition >= StartPosition && NewPosition <= MaximumPosition)
			{
				auto Entry = PositionToCandidate.find(NewPosition);
				if (Entry == PositionToCandidate.end())
				{
					Candidate* NewCandidate = new Candidate(InMove, InParent, NewPosition, InParent->Steps + 1, InForward);
					PositionToCandidate.insert(std::make_pair(NewPosition, NewCandidate));
					NextCandidates.push_back(NewCandidate);
				}
			}
		}
	};

	while (!NextCandidates.empty())
	{
		const std::vector<Candidate*> CurrentCandidates = NextCandidates;
		NextCandidates.clear();
		for (Candidate* CurrentCandidate : CurrentCandidates)
		{
			f32 Position = CurrentCandidate->Position;
			if (Destinations.find(Position) != Destinations.end())
			{
				std::list<Candidate*> Ordered;
				Candidate* Node = CurrentCandidate;
				while (Node != nullptr)
				{
					Ordered.push_front(Node);
					Node = Node->Parent;
				}
				for (Candidate* Node : Ordered)
				{
					std::cout << "#" << Node->Steps << " " << (Node->WhichMove ? Node->WhichMove->Name : "START") << (Node->Forward ? " (forward)" : " (backward)") << " @ " << Node->Position << " (0x" << std::hex << FloatToHex(Node->Position) << std::dec << ")\n";
				}
				std::cout << "\n";
				system("pause");
			}

			for (const Move* Move : MoveSet)
			{
				AddCandidate(Position, Move, true, CurrentCandidate);
				if (Move->BiDirectional)
				{
					AddCandidate(Position, Move, false, CurrentCandidate);
				}
			}
		}
	}

	std::cout << "Exhausted all " << Tested << " possibilities!";
}