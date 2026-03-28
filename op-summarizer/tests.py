import unittest
import subprocess
import json

from dataclasses import asdict
from typing import Dict

from opfinderreader import UniqueOperation
from opsummarizer import OpSummarizer


class Compiler:
    def __init__(self, root_file: str) -> None:
        self.root_file = root_file
        self.gcov_file = f"./{root_file}_gcov.json"
        self.opfinder_file = f"./{root_file}_opfinder.json"

    def compile_and_profile(self) -> None:
        output_file = f"{self.root_file}.out"
        input_file = f"{self.root_file}.c"

        subprocess.call(["gcc",
                         "-fprofile-arcs",
                         "-ftest-coverage",
                         "-o", output_file,
                         input_file])

        subprocess.call([f"./{output_file}"])

        subprocess.call(["gcovr",
                         "-r", ".",
                         "--json",
                         "--output", self.gcov_file])

    def find_operations(self) -> None:
        input_file = f"{self.root_file}.c"

        subprocess.call(["./op-finder",
                         "-o", self.opfinder_file,
                         input_file])


class SummarizerCreatesExpectedOutput(unittest.TestCase):
    def __init__(self, test_name, file_name) -> None:
        super(SummarizerCreatesExpectedOutput, self).__init__(test_name)
        self.root_file_name = file_name
        self.compiler = Compiler(file_name)

    def setUp(self) -> None:
        self.compiler.compile_and_profile()
        self.compiler.find_operations()

    def _get_etalon(self) -> Dict:
        filename = f"{self.root_file_name}_expected.json"
        with open(filename, "r") as f:
            return json.load(f)[f"{self.root_file_name}.c"]

    def test_summarizer_output(self) -> None:
        summarizer = OpSummarizer(self.compiler.gcov_file, self.compiler.opfinder_file)
        found = summarizer.count_operations(f"{self.root_file_name}.c")
        found = summarizer.operation_count_to_json_dict(found)

        etalon = self._get_etalon()

        self.assertEqual(found, etalon, msg="Found operations doesn't match etalon dictionary.")


if __name__ == "__main__":
    suite = unittest.TestSuite()

    suite.addTest(SummarizerCreatesExpectedOutput("test_summarizer_output", "matrix"))
 #   suite.addTest(SummarizerCreatesExpectedOutput("test_summarizer_output", "gauss_blur"))
 #   suite.addTest(SummarizerCreatesExpectedOutput("test_summarizer_output", "for_loop"))
 #   suite.addTest(SummarizerCreatesExpectedOutput("test_summarizer_output", "fir"))

    unittest.TextTestRunner(verbosity=2).run(suite)
