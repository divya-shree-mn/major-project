import json

from dataclasses import asdict
from typing import Dict, List, Tuple

from gcovreader import GCovFile
from opfinderreader import OperationLogReader, UniqueOperation, OperationLog


class OpSummarizer:
    def __init__(self, gcov_path: str, opfinder_path: str) -> None:
        self.gcov = GCovFile(gcov_path)
        self.ops = OperationLogReader(opfinder_path)

        self.gcov.read()
        self.ops.read()

        self.model: Dict[UniqueOperation, Tuple[float, float]] = None
        self.uncounted_estimates: Dict[UniqueOperation, int] = {}
        self.energy_consumption_estimation = 0.0
        self.execution_time_estimation = 0.0

    def add_model(self, model_path: str) -> None:
        with open(model_path, "r") as model_file:
            data = json.load(model_file)

            assert type(data) == list, "Model file did not contain a JSON list."

        self.model = {}
        for entry in data:
            unique_op = OperationLog(**entry["unique_operation"]).entry
            energy = entry["energy_consumption"]
            time = entry["execution_time"]

            self.model[unique_op] = [energy, time]

    def count_operations(self, file: str) -> Dict[UniqueOperation, int]:
        if file not in self.gcov.files or file not in self.ops.files:
            print(f"Gcov files: {self.gcov.files.keys()}")
            print(f"Opfinder files: {self.ops.files.keys()}")
            raise RuntimeError(f"File {file} not in both parsers.")

        op_counter: Dict[UniqueOperation, int] = {}

        for gcov_line in self.gcov.files[file]:
            op_lines = self.ops.get_lines(file, gcov_line.line_number)
            for op_log in op_lines:
                # TODO: revise this. Need a special case for for-loop clauses
                # or branching in general.
                if op_log.branch_number != gcov_line.branch_number:
                    continue

                unique_op = op_log.entry

                if unique_op in op_counter:
                    op_counter[unique_op] += gcov_line.count
                else:
                    op_counter[unique_op] = gcov_line.count

        return op_counter

    def update_estimations(self, uops: Dict[UniqueOperation, int]) -> None:
        assert self.model, "Model not populated."
        for unique_op, op_count in uops.items():
            if unique_op in self.model:
                energy, time = self.model[unique_op]
                self.energy_consumption_estimation += energy * op_count
                self.execution_time_estimation += time * op_count
            elif unique_op in self.uncounted_estimates:
                self.uncounted_estimates[unique_op] += op_count
            else:
                self.uncounted_estimates[unique_op] = op_count

    @staticmethod
    def operation_count_to_json_dict(unique_ops: Dict[UniqueOperation, int]) -> List[Dict]:
        out = []

        for uo, uo_count in unique_ops.items():
            d = asdict(uo)
            d["count"] = uo_count
            out.append(d)

        return out


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Merges gcovr and op-finder outputs.")
    parser.add_argument("files", metavar="FILES", type=str, nargs="+",
                        help="The files to accumulate.")
    parser.add_argument("--gcov", type=str, default="./data/gcov.json",
                        help="The gcovr json file to use.")
    parser.add_argument("--finder", type=str, default="./data/opfinder.json",
                        help="The op-finder json file to use.")
    parser.add_argument("--output", type=str, default=None, required=False,
                        help="The file to output the data to.")
    parser.add_argument("--model", type=str, default=None, required=False,
                        help="The JSON file containing the system model.")
    args = parser.parse_args()

    summarizer = OpSummarizer(args.gcov, args.finder)

    if args.model:
        summarizer.add_model(args.model)

    uops_dictionary = {}
    total_num = 0

    for file_name in args.files:
        ops = summarizer.count_operations(file_name)
        uops_dictionary[file_name] = summarizer.operation_count_to_json_dict(ops)

        if args.model:
            summarizer.update_estimations(ops)

        print(f"Unique operations for file {file_name}:")
        for uop, count in ops.items():
            print(f"\t{count}: {uop}")
            total_num += count

        print("---------")

    print(f"Total count: {total_num}")

    if args.output:
        with open(args.output, "w") as outfile:
            json.dump(uops_dictionary, outfile)

    if args.model:
        power_usage_estimate = summarizer.energy_consumption_estimation
        time_estimate = summarizer.execution_time_estimation
        print("---------")
        print(f"Total energy usage estimation: {power_usage_estimate} mJ\nTotal execution time estimation: {time_estimate} ms")
        if summarizer.uncounted_estimates:
            print("Operations not accounted for (missing from the model):")
            for uop, count in summarizer.uncounted_estimates.items():
                print(f"\t{uop}: {count}")
        else:
            print("No operations missing from the model.")
