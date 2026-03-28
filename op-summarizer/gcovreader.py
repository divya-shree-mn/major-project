import json

from dataclasses import dataclass
from typing import Dict, List


@dataclass
class GCovLine:
    line_number: int
    count: int
    branch_number: int = 0


class GCovFile:
    def __init__(self, path: str) -> None:
        self._path = path
        self._data: dict = {}

        self.files: Dict[str, List[GCovLine]] = {}

    def read(self) -> None:
        with open(self._path, "r") as infile:
            self._data = json.load(infile)

        for file in self._data["files"]:
            name: str = file["file"]
            lines: List[GCovLine] = []

            for line in file["lines"]:
                # Branch specific identification. TODO! later.
                lines.append(GCovLine(line["line_number"], line["count"]))
                if len(line["branches"]):
                    # GCov reports branches in reverse order to our parser.
                    branch_number = len(line["branches"])
                    for branch in line["branches"]:
                        lines.append(GCovLine(line["line_number"], branch["count"], branch_number))
                        branch_number -= 1

            self.files[name] = lines
