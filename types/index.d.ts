// SPDX-License-Identifier: MIT
// TypeScript Version: 3.1

export type JsonnetPrimitiveValue = null | boolean | number | string;

export class Jsonnet {
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
  extString(key: string, value: string): this;

  /**
   * Binds external variable named as `key` to a value represented by `value`.
   *
   * @param key - Name of the external variable
   * @param value - Code representation of the value to which the variable is bound
   */
  extCode(key: string, value: string): this;

  /**
   * Binds top-level argument named as `key` to string `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - String value to which the argument is bound
   */
  tlaString(key: string, value: string): this;

  /**
   * Binds top-level argument named as `key` to a value represented by `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - Code representation of the value to which the argument is bound
   */
  tlaCode(key: string, value: string): this;

  /**
   * Adds a path to the list of include paths. Paths added more recently take precedence.
   *
   * @param path - Path to add.
   */
  addJpath(path: string): this;

  /**
   * Registers a native callback function.
   *
   * @example
   * ```typescript
   * const jsonnet = new Jsonnet();
   * jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b");
   * jsonnet.evaluateSnippet('std.native("add")(2, 3)'); // => 5
   * ```
   *
   * @param name - Name of the callback function.
   * @param fun - The function to register as a callback. `fun` must be a pure function or it must not have any side effects.
   * @param params - Names of the function parameters. `params` must have the equal number of elements as the number of the parameters of the function `fun`.
   */
  nativeCallback<T extends string[]>(
    name: string,
    fun: (...args: JsonnetPrimitiveValue[] & { [K in keyof T]: JsonnetPrimitiveValue }) => any,
    ...params: T): this;

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
