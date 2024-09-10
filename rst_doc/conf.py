# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'DICE'
copyright = '2024, DICE Open Source Contributors'
author = 'DICE Open Source Contributors'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

# Add the extension
extensions = [
    "sphinx.ext.autosectionlabel",
]

# Make sure the target is unique
autosectionlabel_prefix_document = True

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'conestack'
html_theme_options = {
    'cs_color': '#FFFFFF',
    'cs_bg_color': 'var(--bs-indigo)'
}
html_theme_options = {
    'logo_url': '/_static/noun-dice-139343.svg',
    'logo_title': 'DICE',
    'logo_width': '40px',
    'logo_height': '40px'
}

html_static_path = ['_static']



# -- Options for LaTeX output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-latex-output

latex_engine = 'lualatex'
latex_elements = {
    'geometry': r'\usepackage[paperwidth=180mm,paperheight=235mm,left=2cm,right=2cm,top=2cm,bottom=2cm]{geometry}',
    'fncychap': r'\usepackage[Sonny]{fncychap}',
    'fontpkg': r'''
\setmainfont{Bitter-Light}
\setsansfont{Titillium_Web}
''',
}
# doesn't understand svg... latex_logo = '_static/noun-dice-139343.svg'
