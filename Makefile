current_time=$(shell date +%Y%m%d%H%M)
# 获得当前git的用户邮箱
current_user=$(shell git config user.email)

.PHONY:clean cc

all: clean

cc:
	git rm -r --cached .
	git add .
	git commit -m "$(current_user) batch push $(current_time)"
	git push origin develop

clean:
	cmd /c keilkill.bat


