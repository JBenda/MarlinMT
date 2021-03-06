
## The Marlin documentation

The Marlin documentation is available at: https://iLCSoft.github.io/Marlin/

The documentation format is in markdown and managed with mkdocs.

The documentation sources are only available on the `marlin-mt` branch of the project and are deployed on the `gh-pages` branch on github.

## Installing development tools

Install doxygen with your package manager, e.g:

```shell
sudo apt-get install doxygen
```

Install `mkdocs` using `pip`:

```shell
pip install mkdocs
```

To generate correctly the doxygen documentation and deploy it with mkdocs, you need a mkdocs plugin.

To install it:

```shell
git clone https://github.com/pieterdavid/mkdocs-doxygen-plugin.git
cd mkdocs-doxygen-plugin
python setup.py build install
```

You are ready to develop the documentation.


### Deploy the documentation

If you develop on the documentation, you will need to install the `mkdocs` command locally to deploy the documentation either locally or on github.

To deploy it locally:

```shell
cd Marlin
mkdocs build
mkdocs serve
```

Your doc is deployed by default at `http://127.0.0.1:8000`. Type `mkdocs --help` to see options.

To deploy your docs on github using your Marlin fork. Assuming your remote name for your fork is `fork`, you can deploy your doc on the gihub servers:

```shell
cd Marlin
mkdocs gh-deploy -r fork
```

Your doc will be deploy at `https://username.github.io/Marlin/`.

If you are admin in the iLCSoft collaboration or have the push access on the `iLCSoft/Marlin` repository, you can also deploy the doc:

```shell
cd Marlin
mkdocs gh-deploy
```
