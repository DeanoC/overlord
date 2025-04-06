import os
import toml
import yaml
import argparse
from toml import TomlDecodeError
from toml.decoder import InlineTableDict

def convert_inline_table_dict(data):
    if isinstance(data, InlineTableDict):
        return dict(data)
    elif isinstance(data, dict):
        return {k: convert_inline_table_dict(v) for k, v in data.items()}
    elif isinstance(data, list):
        return [convert_inline_table_dict(i) for i in data]
    else:
        return data

def convert_toml_to_yaml(toml_file_path, yaml_file_path):
    try:
        with open(toml_file_path, 'r') as toml_file:
            toml_data = toml.load(toml_file)
        
        # Convert InlineTableDict to regular dict
        toml_data = convert_inline_table_dict(toml_data)
        
        with open(yaml_file_path, 'w') as yaml_file:
            yaml.dump(toml_data, yaml_file, default_flow_style=False)
        print(f"Successfully converted {toml_file_path} to {yaml_file_path}")
        
        # Delete the TOML file after successful conversion
        os.remove(toml_file_path)
        print(f"Deleted original TOML file: {toml_file_path}")
    except TomlDecodeError as e:
        print(f"Error decoding TOML file {toml_file_path}: {e}")
    except Exception as e:
        print(f"Unexpected error processing {toml_file_path}: {e}")

def find_and_convert_toml_files(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.toml'):
                print(f'Found TOML file: {file}')
                toml_file_path = os.path.join(root, file)
                yaml_file_path = toml_file_path.replace('.toml', '.yaml')
                convert_toml_to_yaml(toml_file_path, yaml_file_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert TOML files to YAML files in a specified directory.")
    parser.add_argument("path", type=str, help="Path to the directory containing TOML files.")
    args = parser.parse_args()

    find_and_convert_toml_files(args.path)