# coding: utf-8

lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'digest/xxhash/version'

Gem::Specification.new do |spec|
  spec.name          = "digest-xxhash"
  spec.version       = Digest::XXHash::VERSION
  spec.authors       = ["konsolebox"]
  spec.email         = ["konsolebox@gmail.com"]
  spec.summary       = "A Digest framework based XXHash library for Ruby"
  spec.description   = <<-EOF
    This gem provides XXH32, XXH64, XXH3_64bits and XXH3_128bits
    functionalities for Ruby.  It inherits Digest::Class and complies
    with Digest::Instance's functional design.
  EOF
  spec.homepage      = "https://github.com/konsolebox/digest-xxhash-ruby"
  spec.license       = "MIT"

  spec.required_ruby_version = '>= 2.2'

  spec.files         = `git ls-files -z`.split("\x0").reject{ |f| f =~ /\.yml$/ }
  spec.executables   = spec.files.grep(%r{^bin/}){ |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler", "~> 1.0", "!= 1.1.3", "!= 1.1.4", "!= 1.1.5"
  spec.add_development_dependency "minitest", "~> 5.8"

  spec.extensions = %w[ext/digest/xxhash/extconf.rb]
end
