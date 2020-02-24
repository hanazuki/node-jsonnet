// SPDX-License-Identifier: MIT
// TypeScript Version: 3.0

declare class Jsonnet {
  /**
   * Returns the version of linked libjsonnet.
   */
  static readonly version: string;

  /**
   * Constructs a new Jsonnet interpreter.
   */
  constructor();

  /**
   * Binds external variable named as `key` to string `value`.
   *
   * @param key - Name of the external variable
   * @param value - String value to which the variable is bound
   */
  extString(key: string, value: string): void;

  /**
   * Binds external variable named as `key` to a value represented by `value`.
   *
   * @param key - Name of the external variable
   * @param value - Code representation of the value to which the variable is bound
   */
  extCode(key: string, value: string): void;

  /**
   * Binds top-level argument named as `key` to string `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - String value to which the argument is bound
   */
  tlaString(key: string, value: string): void;

  /**
   * Binds top-level argument named as `key` to a value represented by `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - Code representation of the value to which the argument is bound
   */
  tlaCode(key: string, value: string): void;

  /**
   * Adds a path to the list of include paths. Paths added more recently take precedence.
   *
   * @param path - Path to add.
   */
  addJpath(path: string): void;

  /**
   * Evaluates a Jsonnet script in a file.
   *
   * @param filename - Path to the file.
   * @returns The evaluation result in JSON representation.
   */
  evaluateFile(filename: string): Promise<string>;

  /**
   * Evaluates a Jsonnet script give as a string.
   *
   * @param snippet - Jsonnet script to evaluate.
   * @param filename - Filename used for error reporting.
   * @returns The evaluation result in JSON representation.
   */
  evaluateSnippet(snippet: string, filename?: string): Promise<string>;
}
