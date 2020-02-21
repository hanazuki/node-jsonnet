{
  "targets": [
    {
      "target_name": "node-jsonnet",
      "sources": [
        "src/binding.cc",
      ],
      "include_dirs": [
        '<!(node -e "require(\'nan\')")',
      ],
    }
  ]
}
