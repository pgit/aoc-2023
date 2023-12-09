#
# https://github.com/microsoft/vscode-dev-containers/tree/main/containers/debian
#
FROM docker.io/psedoc/cpp-devcontainer:0.1

#
# enable persistent bash history in container
#
# https://code.visualstudio.com/remote/advancedcontainers/persist-bash-history
#
ARG USERNAME=vscode
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R $USERNAME /commandhistory \
    && echo "$SNIPPET" >> "/home/$USERNAME/.bashrc"

SHELL ["/bin/bash", "-c"]
