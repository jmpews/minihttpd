set number
set autoindent
set smartindent
set showmatch
set ruler
set tabstop=4
set expandtab
syntax enable

"colorscheme Tomorrow-Night-Eighties
colorscheme onedark

set guifont=Monaco:h12 " 设置字体
set encoding=utf-8

"if filereadable(expand("~/.vimrc.bundles"))
"source ~/.vimrc.bundles
"endif

" Display extra whitespace
set list listchars=tab:»·,trail:·

" Quicker window movement
nnoremap <C-j> <C-w>j
nnoremap <C-k> <C-w>k
nnoremap <C-h> <C-w>h
nnoremap <C-l> <C-w>l

" Nerd Tree
let NERDChristmasTree=0
let NERDTreeWinSize=40
let NERDTreeChDirMode=2
let NERDTreeIgnore=['\~$', '\.pyc$', '\.swp$']
let NERDTreeShowBookmarks=1
let NERDTreeWinPos="left"
autocmd vimenter * if !argc() | NERDTree | endif " Automatically open a NERDTree if no files where specified
autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTreeType") && b:NERDTreeType == "primary") | q | endif " Close vim if the only window left open is a NERDTree
nmap <F5> :NERDTreeToggle<cr>
