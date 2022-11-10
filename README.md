# Earth Tile Server
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FShtille%2Fearth-tileserver.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2FShtille%2Fearth-tileserver?ref=badge_shield)


*Earth Tile Server* is the tile server made for [Earth WebGL](https://github.com/Shtille/earth-webgl).
It parses GET requests for coordinates and generates images (tiles) for response.

## Dependencies
* [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)
* [curl](https://curl.se/)
* gcc/clang/mingw

## Building
1. Install all dependencies.
2. Build via make command.

## Deploying
### Install
To install sources on Ubuntu 18.04 use following script:
```bash
sudo apt update
sudo apt install git
sudo apt install gcc
sudo apt install libcurl4-openssl-dev
sudo apt install libmicrohttpd-dev
PATH=/usr/include/x86_64-linux-gnu:$PATH
mkdir dev
cd dev
git clone https://github.com/Shtille/earth-tileserver.git earth-tileserver
cd earth-tileserver
git submodule init
git submodule update
```

### Run binary
To run the binary _earth-tileserver.app_ use following script with chosen port number to listen:
```bash
cd bin
./earth-tileserver.app %PORT%
```

## Testing
Use *test.html* as test browser page for tiles loading.

## License
Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).


[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FShtille%2Fearth-tileserver.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2FShtille%2Fearth-tileserver?ref=badge_large)

## Bug Reporting
Please log bugs under [Issues](https://github.com/Shtille/earth-tileserver/issues) on github.

## Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.