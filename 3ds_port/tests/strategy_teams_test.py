#!/usr/bin/env python3
"""Check the hand-crafted strategy teams (src/3ds_strategy_teams.h) against a tree.

Run after bootstrap: python3 3ds_port/tests/strategy_teams_test.py --tree build/upstream

Every species, nature, item, move and trainer must be one of the tree's
constants (the build checks that too); beyond it, each Pokemon's named
ability must be one of its species' two, no team repeats a move on one
Pokemon, has more than six Pokemon or is listed twice, and levels stay
1..100. No game data is read beyond the decompilation's own headers.
"""
import argparse
import re
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
TEAMS = HERE.parent / "src" / "3ds_strategy_teams.h"
TREE = HERE.parents[1]

MON = re.compile(r"MON\((\w+),\s*(\d+),\s*(\w+),\s*(\w+),\s*(\w+),\s*(\d+),\s*([\w(), ]+?),"
                 r"\s*(\w+),\s*(\w+),\s*(\w+),\s*(\w+)\)")


def constants(path, prefix):
    return set(re.findall(r"#define (%s\w+)" % prefix, (TREE / path).read_text(encoding="utf-8")))


def species_abilities():
    info = (TREE / "src/data/pokemon/species_info.h").read_text(encoding="utf-8")
    parts = re.split(r"\n    \[SPECIES_(\w+)\] =", info)
    abilities = {}
    for name, body in zip(parts[1::2], parts[2::2]):
        found = re.search(r"\.abilities = \{(\w+), (\w+)\}", body)
        if found:
            abilities[name] = found.groups()
    return abilities


class StrategyTeamTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.text = TEAMS.read_text(encoding="utf-8")
        cls.arrays = dict(re.findall(r"static const struct CtrStrategyMon (sStrat_\w+)\[\] = \{(.*?)\n\};",
                                     cls.text, re.S))
        cls.teams = re.findall(r"TEAM\((\w+), (sStrat_\w+)\)", cls.text)

    def test_every_team_is_listed_once_and_exists(self):
        trainers = constants("include/constants/opponents.h", "TRAINER_")
        self.assertTrue(self.teams)
        self.assertEqual(len({t for t, _ in self.teams}), len(self.teams))
        for trainer, array in self.teams:
            with self.subTest(trainer=trainer):
                self.assertIn("TRAINER_" + trainer, trainers)
                self.assertIn(array, self.arrays)
        self.assertEqual(set(self.arrays), {a for _, a in self.teams})

    def test_rows(self):
        species = constants("include/constants/species.h", "SPECIES_")
        moves = constants("include/constants/moves.h", "MOVE_")
        items = constants("include/constants/items.h", "ITEM_")
        natures = constants("include/constants/pokemon.h", "NATURE_")
        abilities = constants("include/constants/abilities.h", "ABILITY_")
        own = species_abilities()
        for array, body in self.arrays.items():
            rows = MON.findall(body)
            self.assertEqual(len(rows), body.count("MON("), array)
            self.assertTrue(1 <= len(rows) <= 6, array)
            for name, level, nature, ability, item, iv, _evs, *row_moves in rows:
                with self.subTest(team=array, species=name):
                    self.assertIn("SPECIES_" + name, species)
                    self.assertTrue(1 <= int(level) <= 100)
                    self.assertTrue(0 <= int(iv) <= 31)
                    self.assertIn("NATURE_" + nature, natures)
                    self.assertIn("ITEM_" + item, items)
                    self.assertIn("ABILITY_" + ability, abilities)
                    self.assertIn("ABILITY_" + ability, own[name], "not one of its abilities")
                    for move in row_moves:
                        self.assertIn("MOVE_" + move, moves)
                    self.assertEqual(len(set(row_moves)), 4, "a move twice")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tree", type=Path, default=TREE)
    args, rest = parser.parse_known_args()
    TREE = args.tree.resolve()
    unittest.main(argv=[sys.argv[0]] + rest)
