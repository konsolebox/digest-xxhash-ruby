# coding: utf-8

lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'digest/xxhash/version'

Gem::Specification.new do |spec|
  spec.name          = "digest-xxhash"
  spec.version       = Digest::XXHash::VERSION
  spec.authors       = ["konsolebox"]
  spec.email         = ["konsolebox@gmail.com"]
  spec.summary       = "XXHash for Ruby"
  spec.description   = "An XXHash library that complies with Digest::Instance's functional design."
  spec.homepage      = "https://github.com/konsolebox/digest-xxhash-ruby"
  spec.license       = "MIT"

  spec.required_ruby_version = '>= 2.2'

  spec.files         = `git ls-files -z`.split("\x0").reject{ |f| f =~ /\.yml$/ }
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler", "~> 1.0"
  spec.add_development_dependency "minitest", "~> 5.8"

  spec.extensions = %w[ext/digest/xxhash/extconf.rb]
end
