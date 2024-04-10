/*
*    DKU Operating System Lab
*    Lab1 (Scheduler Algorithm Simulator)
*    Student id : 32221741 
*    Student name : Park Seonghyeon 
*/

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include "sched.h"

// Shortest Process Next (Shortest Job First)
class SPN : public Scheduler{
    private:
    // 하나의 작업이 완료될 때, job_queue_에서 arrival time이 current time 이하인 job들을
    // 뽑아와 벡터 v에 저장
    std::vector<Job> v;
    
    // v에서 service time이 긴 순서대로 정렬
    // service time이 같다면 name 큰 순서로 정렬
    // pop_back()시 shortest job이 반환된다.
    static bool compare(const Job& j1, const Job& j2) {
        if (j1.service_time == j2.service_time) {
            return j1.name > j2.name;
        } else {
            return j1.service_time > j2.service_time;
        }
    }
    public:
        SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SPN";
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }

            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (v.empty() && job_queue_.empty()) return -1;

                // 새로운 작업 할당
                // 현재까지 도착한 작업을 저장하여 짧은 service time이 뒤로 가도록 정렬
                while (!job_queue_.empty()) {
                    Job front = job_queue_.front();
                    if (front.arrival_time <= current_time_) {
                        v.push_back(front);
                        job_queue_.pop();
                    } else break;
                }
                sort(v.begin(), v.end(), compare);
                // 가장 짧은 service time을 가지는 작업을 수행
                current_job_ = v.back();
                v.pop_back();
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

// Round Robin (time quantum = time_slice)
class RR : public Scheduler{
    private:
        int time_slice_; // time quantum
        int left_slice_; // 남은 slice 시간 = time quantum - 현재 작업으로 문맥교환 이후 지난 시간
        std::queue<Job> waiting_queue; // 도착한 작업이 기다리는 대기 큐
    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            // 도착한 작업을 도착 순으로 대기 큐에 push
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    waiting_queue.push(front);
                    job_queue_.pop();
                } else break;
            }
            // time quantum 시간이 지나거나 현재 작업이 끝나면 문맥 교환 시도해야함
            if (left_slice_ == 0 || current_job_.remain_time == 0) {
                // 현재 작업이 끝났을 시
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);
                    // 남은 작업이 없으면 종료
                    if (waiting_queue.empty() && job_queue_.empty()) return -1;
                } else if (!waiting_queue.empty()) {
                    // 현재 작업이 끝나지 않았고 대기 큐가 비어있지 않으면 대기 큐에 push
                    // 대기 큐가 비어있으면 push할 필요 X
                    waiting_queue.push(current_job_);
                }
                // 위에서 push 안했으면 false, 문맥 교환이 필요 X
                if (!waiting_queue.empty()) {
                    // 문맥 교환 후 switch_time_을 현재 시간에 추가
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();
                    current_time_ += switch_time_;
                }
                
                // left slice을 time slice 값으로 초기화
                left_slice_ = time_slice_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            // 남은 slice 시간 update
            left_slice_--;
            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

// Shortest Remaining Time (Shortest Remaining Job First)
class SRT : public Scheduler{
    private:
        // SPN과 같이 현재 도착한 작업을 저장할 벡터 v와
        // remain time이 긴 순대로 정렬하는 compare
        std::vector<Job> v;
        static bool compare (const Job& j1, const Job& j2) {
            if (j1.remain_time == j2.remain_time) {
                return j1.name > j2.name;
            } else {
                return j1.remain_time > j2.remain_time;
            }
        }
    public:
        SRT(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SRT";
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            // 도착한 작업 저장
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    v.push_back(front);
                    job_queue_.pop();
                } else break;
            }
            // 도착한 작업들을 remain time 긴 순으로 정렬
            sort(v.begin(), v.end(), compare);
            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);
                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && v.empty()) return -1;
                // 대기열에 작업 존재 시 그 작업으로 문맥교환
                if (!v.empty()) {
                    current_job_ = v.back();
                    v.pop_back();
                    current_time_ += switch_time_;
                }
            // 작업이 끝나지 않았지만 도착한 작업 중 remain time이 더 짧은게 있다면
            } else if (!v.empty() && v.front().remain_time < current_job_.remain_time) {
                Job temp = current_job_;
                current_job_ = v.back();
                v.pop_back();
                v.push_back(temp);
                current_time_ += switch_time_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            return current_job_.name;
        }
};
// Highest Response Ratio Next
class HRRN : public Scheduler{
    private:
        // 작업마다 대기 시간을 기록해 놓기 위해 구조체 작성
        struct Waiting_job {
            Job job;
            double waiting_time;
        };
        // 구조체 벡터에서 대기열 관리
        std::vector<Waiting_job> v;
        // Response Ratio = (waiting time + service time) / service time
        // Response Ratio가 높은 순으로 스케줄링 되어야 함
        static bool compare (const Waiting_job& w1, const Waiting_job& w2) {
            double rr1 = (w1.waiting_time+w1.job.service_time)/w1.job.service_time;
            double rr2 = (w2.waiting_time+w2.job.service_time)/w2.job.service_time;
            if (rr1 == rr2) {
                return w1.job.name > w2.job.name;
            } else {
                return rr1 < rr2;
            }
        }
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
        }
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            // 도착한 작업으로 구조체 Waiting_job 한 개를 만들고 push
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    Waiting_job temp;
                    temp.job = front;
                    temp.waiting_time = current_time_ - temp.job.arrival_time;
                    v.push_back(temp);
                    job_queue_.pop();
                } else break;
            }
            // waiting time 업데이트
            for (int i = 0; i < v.size(); i++) {
                v[i].waiting_time = current_time_ - v[i].job.arrival_time;
            }
            // 현재 작업을 모두 완료할 때
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);
                // 모든 작업이 끝나면 -1 리턴
                if (v.empty() && job_queue_.empty()) return -1;
                // Response Ratio가 높은 작업을 back()으로 끌어오기 위해 정렬
                sort(v.begin(), v.end(), compare);
                // 문맥 교환
                current_job_ = v.back().job;
                v.pop_back();
                current_time_ += switch_time_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            return current_job_.name;
        }
};

// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler{
    private:
        // 각 작업마다 time quantum을 가지고 그 값은 is_2i가
        // false면 1이고 true면 2^i다. (i = 0,1,2...며 문맥 교환시 1씩 증가)
        struct job_with_lt {
            Job job;
            int left_slice;
        };
        // 구조체를 4개의 레벨의 큐로 관리
        // 인덱스가 낮을 수록 높은 우선순위
        std::queue<job_with_lt> queues[4];
        bool is_2i_;
        int time_slice_;
        int left_slice_;
        int curLevel; // 현재 수행 중인 작업이 멀티 레벨 큐에 어떤 레벨(인덱스)인지 저장하는 변수
        // 멀티 레벨 큐 전체가 비어있는지 체크하는 함수
        bool isEmpty() {
            for (int i = 0; i < 4; i++) {
                if (!queues[i].empty()) {
                    return false;
                }
            }
            return true;
        }
    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }
            is_2i_ = is_2i;
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                time_slice_ = 1; // is_2i 여부 상관 없이 초기값은 1
                left_slice_ = time_slice_;
                curLevel = 0;
                job_queue_.pop();
            }
            // 도착한 작업을 상위 큐(인덱스 0)에 채우기
            while(!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    job_with_lt temp;
                    temp.job = front;
                    temp.left_slice = 1;
                    queues[0].push(temp);
                    job_queue_.pop();
                } else break;
            }
            // 현재 작업 완료 혹은 time_slice_가 0이면 문맥 교환
            if (current_job_.remain_time == 0 || left_slice_ <= 0) {
                // 현재 작업을 큐에 push할 때 사용할 cur
                job_with_lt cur;
                cur.job = current_job_;
                // 2i 옵션이 활성화 시 현재 작업의 다음 time_slice_는 두 배다.
                cur.left_slice = is_2i_ ? time_slice_*2 : time_slice_;
                // 현재 작업을 마쳤으면
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);
                    // 모든 작업이 끝나면 -1 리턴
                    if (isEmpty() && job_queue_.empty()) return -1;
                } else if (!isEmpty()) { // 초기에 이 작업만 수행 중이면 굳이 push할 이유는 없다.
                    // push 할 때는 현재 레벨보다 1 높아야한다.
                    // 현재 레벨이 4면 4에다가 그대로 push
                    if (curLevel < 3) queues[curLevel+1].push(cur);
                    else queues[curLevel].push(cur);
                }
                // 가장 높은 우선순위의 큐의 작업을 pop해서 문맥교환한다.
                if (!isEmpty()) {
                    for (int i = 0; i < 4; i++) {
                        if (!queues[i].empty()) {
                            job_with_lt front = queues[i].front();
                            current_job_ = front.job;
                            time_slice_ = front.left_slice;
                            left_slice_ = time_slice_;
                            curLevel = i;
                            queues[i].pop();
                            // 혹여나 pop한 작업이 원래 하던 작업이었으면 문맥 교환이 아니므로
                            // switch_time_을 더하면 안된다.
                            if (current_job_.name != cur.job.name) current_time_ += switch_time_;
                            break;
                        }
                    }
                }
            }
            
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            left_slice_--; // 남은 time quantum 시간 업데이트
            return current_job_.name;
        }
};
