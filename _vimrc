set encoding=utf-8
scriptencoding utf-8
lcd ~
" 無名レジスタに入るデータを、*レジスタにも入れる。
set clipboard+=unnamed
" undo履歴を記録
set undofile
" 行番号の表示
set number
" タブを挿入するときの幅
set tabstop=4
" タブを表示するときの幅
set shiftwidth=4
" タブをタブとして扱う(スペースに展開しない)
"set noexpandtab
set softtabstop=0
" タブ、空白、改行の可視化
set list
set listchars=tab:»»,trail:␣,eol:$,extends:»,precedes:«,nbsp:%
" 補完ポップアップメニューの行数
set pumheight=10
" 全角スペースをハイライト表示
function! ZenkakuSpace()
    highlight ZenkakuSpace cterm=none ctermbg=0 gui=none guibg=#282a2e
endfunction
if has('syntax')
    augroup ZenkakuSpace
        autocmd!
        autocmd ColorScheme       * call ZenkakuSpace()
        autocmd VimEnter,WinEnter * match ZenkakuSpace /　/
    augroup END
    call ZenkakuSpace()
endif
" 検索で大文字小文字を区別する
set noignorecase

" 表示行単位で上下移動するように
noremap j gj
noremap k gk
noremap <Down> gj
noremap <Up>   gk
inoremap <silent> <Down> <C-o>gj
inoremap <silent> <Up>   <C-o>gk
" 逆に普通の行単位で移動したい時のために逆の map も設定しておく
noremap gj j
noremap gk k
" 「Y」カーソルから行末までをコピーする。
nnoremap Y y$
" Esc Esc でハイライトOFF
nnoremap <Esc><Esc> :<C-u>set nohlsearch<Return>
" エラーメッセージの表示時にビープを鳴らさない
set vb t_vb=
set visualbell
set noerrorbells
" ファイルを開くと、そのファイルと同じディレクトリに移動
:source $VIMRUNTIME/macros/cd.vim

"--------------------------------------------------------------------------
" Vim で C++ の設定例
" filetype=cpp が設定された時に呼ばれる関数
" Vim で C++ の設定を行う場合はこの関数内で記述する
" ここで設定する項目は各自好きに行って下さい
function! s:cpp()
    " インクルードパスを設定する
    " gf などでヘッダーファイルを開きたい場合に影響する
    "setlocal path+=D:/home/cpp/boost,D:/home/cpp/sprout

    "タブ文字の長さ
    setlocal tabstop=4
    setlocal shiftwidth=4

    " 空白文字ではなくてタブ文字を使用する
    setlocal noexpandtab

    " 括弧を構成する設定に <> を追加する。
    setlocal matchpairs+=<:>

    " 行を折り返さない。
    setlocal nowrap

    " 最後に定義された include 箇所へ移動してを挿入モードへ
    "nnoremap <buffer><silent> <Space>ii :execute "?".&include<CR> :noh<CR> o

    " BOOST_PP_XXX 等のハイライトを行う
    "syntax match boost_pp /BOOST_PP_[A-z0-9_]*/
    "highlight link boost_pp cppStatement
endfunction
augroup vimrc-cpp
    autocmd!
    " filetype=cpp が設定された場合に関数を呼ぶ
    autocmd FileType cpp call s:cpp()
augroup END

"--------------------------------------------------------------------------
" ファイルエンコーディングや文字コードをステータス行に表示する。
set laststatus=2
set statusline=%<%f\ %m\ %r%h%w%{'['.(&filetype).']'.(&bomb?'[BOM]':'').'['.(&fenc!=''?&fenc:&enc).']['.&ff.']'}%=\ (%v,%l)/%L%8P\ 

"--------------------------------------------------------------------------
" neobundleの設定。
set nocompatible               " Be iMproved
filetype off                   " Required!

if has('vim_starting')
  set runtimepath+=~/_vim/bundle/neobundle.vim/
endif

" neobundle プラグイン記述の開始。
call neobundle#begin(expand('~/_vim/bundle/'))
NeoBundleFetch 'Shougo/neobundle.vim'

NeoBundle 'Shougo/unite.vim'
let g:unite_data_directory='~/_vim/_cache/unite/'
let g:unite_split_rule = 'rightbelow'

NeoBundle 'Shougo/neomru.vim'
"let g:neomru#file_mru_path='~/_vim/_cache/neomru/file'
let g:neomru#directory_mru_path='~/_vim/_cache/neomru/directory'
" バッファ一覧
nnoremap <silent> ,ub :<C-u>Unite buffer<CR>
" ファイル一覧
nnoremap <silent> ,uf :<C-u>UniteWithBufferDir -buffer-name=files file<CR>
" レジスタ一覧
nnoremap <silent> ,ur :<C-u>Unite -buffer-name=register register<CR>
" 最近使用したファイル一覧
nnoremap <silent> ,um :<C-u>Unite file_mru<CR>
" 常用セット
nnoremap <silent> ,uu :<C-u>Unite buffer file_mru<CR>
" 全部乗せ
nnoremap <silent> ,ua :<C-u>UniteWithBufferDir -buffer-name=files buffer file_mru bookmark file<CR>

NeoBundle 'soramugi/auto-ctags.vim'

" neobundle プラグイン記述の終了。
call neobundle#end()
filetype plugin indent on     " Required!

" Installation check.
if neobundle#exists_not_installed_bundles()
  echomsg 'Not installed bundles : ' .
        \ string(neobundle#get_not_installed_bundle_names())
  echomsg 'Please execute ":NeoBundleInstall" command.'
  "finish
endif

"--------------------------------------------------------------------------
" 自動改行を禁止
set formatoptions=q
set display=lastline
