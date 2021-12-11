# digest-xxhash

This gem provides XXH32, XXH64, XXH3_64bits and XXH3_128bits
functionalities for Ruby.  It inherits Digest::Class and complies
with Digest::Instance's functional design.

Its core implementation comes from the official source, which is in
https://github.com/Cyan4973/xxHash.

## Installation

Add this line to the application's Gemfile:

    gem 'digest-xxhash'

And then execute:

    $ bundle

It can also be installed manually with:

    $ gem install digest-xxhash

The library can also be installed in Gentoo system-wide using 'layman':

    # Fetch remote list of overlays, and add 'konsolebox' overlay
    layman -f && layman -a konsolebox

    # Unmask unstable keyword
    echo 'dev-ruby/digest-xxhash' > /etc/portage/package.keywords/dev-ruby.digest-xxhash

    # Merge package
    emerge dev-ruby/digest-xxhash

## Example Usage

    require 'digest/xxhash'

    Digest::XXH32.digest("ABXY")
    => "\x0E\xA4\xB6\xCA"

    Digest::XXH32.hexdigest("ABXY")
    => "0ea4b6ca"

    Digest::XXH32.idigest("ABXY")
    => 245675722

    Digest::XXH32.hexdigest("1234", "\xab\xcd\xef\x00")
    => "2af3be26"

    Digest::XXH32.hexdigest("1234", "abcdef00")
    => "2af3be26"

    Digest::XXH32.hexdigest("1234", 2882400000)
    => "2af3be26"

    Digest::XXH64.hexdigest("1234", "0123456789abcdef")
    => "d7544504de216507"

    Digest::XXH64.new("0123456789abcdef").update("1234").hexdigest
    => "d7544504de216507"

    Digest::XXH64.new.reset("0123456789abcdef").update("12").update("34").hexdigest
    => "d7544504de216507"

    Digest::XXH3_64bits.hexdigest("1234", "0123456789abcdef")
    => "4156724c7605b1be"

    Digest::XXH3_64bits.new.reset_with_secret("abcd" * 34).update("1234").hexdigest
    => "f7bbdbf9ec8c6394"

    Digest::XXH3_128bits.hexdigest("1234", "0123456789abcdef") # XXH3_128bits() only allows 64-bit seeds
    => "ad6108fb0b9a6b51b7f80d053c76c0fd"

    Digest::XXH3_128bits.new.reset_with_secret("abcd" * 34).update("1234").hexdigest
    => "0d44dd7fde8ea2b4ba961e1a26f71f21"

## API Documentation

RubyGems.org provides autogenerated API documentation of the library in
https://www.rubydoc.info/gems/digest-xxhash/.

## Homepage

https://rubygems.org/gems/digest-xxhash

## Contributing

1. Fork it ( https://github.com/konsolebox/digest-xxhash-ruby/fork ).
2. Create feature branch (`git checkout -b my-new-feature`).
3. Commit changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin my-new-feature`).
5. Create a new Pull Request.

[![Build Status](https://github.com/konsolebox/digest-xxhash-ruby/actions/workflows/ruby.yml/badge.svg)](https://github.com/konsolebox/digest-xxhash-ruby/actions/workflows/ruby.yml)
[![Build Status](https://ci.appveyor.com/api/projects/status/kb6hvlxjms3ftw7u?svg=true)](https://ci.appveyor.com/project/konsolebox/digest-xxhash-ruby)
