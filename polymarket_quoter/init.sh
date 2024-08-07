if test -f requirements.txt
  then
    echo "here"
    pip install -r ./requirements.txt
  else echo "There's no requirements.txt, so nothing to install."
fi
