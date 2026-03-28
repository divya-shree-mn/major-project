import json

from enum import Enum
from dataclasses import dataclass
from typing import Any, Dict, List, Union


class OperationType(Enum):
    BASIC = "basic_operation"
    FUNCTION_CALL = "function_call"


@dataclass(frozen=True)
class BasicOperation:
    operation_name: str
    type_lhs: str
    type_rhs: str
    type_result: str


@dataclass(frozen=True)
class FunctionCall:
    function_name: str
    call_result_type: str


UniqueOperation = Union[BasicOperation, FunctionCall]


@dataclass
class OperationLog:
    line: int
    branch_number: int
    entry_type: OperationType
    entry: UniqueOperation

    def __post_init__(self) -> None:
        self.entry_type = OperationType(self.entry_type)

        if self.entry_type == OperationType.BASIC:
            self.entry = BasicOperation(**self.entry)
        elif self.entry_type == OperationType.FUNCTION_CALL:
            self.entry = FunctionCall(**self.entry)
        else:
            assert False, "Unaccounted for operation type."


class OperationLogReader:
    def __init__(self, path: str) -> None:
        self._path = path
        self._data: dict = {}

        self.files: Dict[str, List[OperationLog]] = {}

    def read(self) -> None:
        with open(self._path, "r") as infile:
            self._data = json.load(infile)

        for name, ops_list in self._data.items():
            ops: List[OperationLog] = []

            for op_json in ops_list:
                ops.append(OperationLog(**op_json))

            self.files[name] = ops

    def get_lines(self, file: str, line_number: int) -> List[OperationLog]:
        res: List[OperationLog] = []
        for line in self.files[file]:
            if line.line == line_number:
                res.append(line)

        return res
